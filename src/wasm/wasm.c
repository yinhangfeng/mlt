#include <framework/mlt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if (defined(__APPLE__) || defined(_WIN32) || defined(HAVE_SDL2)) && \
    !defined(MELT_NOSDL)
#include <SDL.h>
#endif

#if (defined(__EMSCRIPTEN__))
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

static const int STATE_CREATED = 0;
static const int STATE_INITIALIZING = 1;
static const int STATE_INITIALIZED = 2;
static const int STATE_STOPPED = 3;
static const int STATE_RUNNING = 4;
static const int STATE_RELEASED = 5;

static const int ERROR_INIT = 1;
static const int ERROR_START = 2;
static const int ERROR_FATAL = 3;

// TODO 命名
struct mlt_instance_s {
  mlt_producer producer;
  mlt_consumer consumer;
  mlt_profile profile;
  bool is_consumer_explicit;
  int argc;
  char** argv;
  volatile int state;
  volatile bool is_starting;
  pthread_mutex_t mutex;
  // 0: normal; 1: init error; 2: start error; 3: consumer_fatal_error
  volatile int error;
};

typedef struct mlt_instance_s* mlt_instance;

static mlt_consumer create_consumer(mlt_profile profile, char* id) {
  // id=name:input
  char* name = id ? strdup(id) : NULL;
  char* input = name ? strchr(name, ':') : NULL;
  if (input != NULL) *input++ = '\0';
  mlt_consumer consumer = mlt_factory_consumer(profile, name, input);
  free(name);
  return consumer;
}

static void load_consumer(mlt_consumer* consumer, mlt_profile profile, int argc,
                          char** argv) {
  int i;
  int multi = 0;
  int qglsl = 0;

  for (i = 1; i < argc; i++) {
    // See if we need multi consumer.
    multi += !strcmp(argv[i], "-consumer");
    // Seee if we need the qglsl variant of multi consumer.
    if (!strncmp(argv[i], "glsl.", 5) || !strncmp(argv[i], "movit.", 6))
      qglsl = 1;
  }
  // Disable qglsl if xgl is being used!
  for (i = 1; qglsl && i < argc; i++)
    if (!strcmp(argv[i], "xgl")) qglsl = 0;

  if (multi > 1 || qglsl) {
    // If there is more than one -consumer use the 'multi' consumer.
    int k = 0;
    char key[20];

    if (*consumer) mlt_consumer_close(*consumer);
    *consumer = create_consumer(profile, (qglsl ? "qglsl" : "multi"));
    mlt_properties properties = MLT_CONSUMER_PROPERTIES(*consumer);
    for (i = 1; i < argc; i++) {
      if (!strcmp(argv[i], "-consumer") && argv[i + 1]) {
        // Create a properties object for each sub-consumer
        mlt_properties new_props = mlt_properties_new();
        snprintf(key, sizeof(key), "%d", k++);
        mlt_properties_set_data(properties, key, new_props, 0,
                                (mlt_destructor)mlt_properties_close, NULL);
        if (strchr(argv[i + 1], ':')) {
          char* temp = strdup(argv[++i]);
          char* service = temp;
          char* target = strchr(temp, ':');
          *target++ = 0;
          mlt_properties_set(new_props, "mlt_service", service);
          mlt_properties_set(new_props, "target", target);
        } else {
          mlt_properties_set(new_props, "mlt_service", argv[++i]);
        }
        while (argv[i + 1] && strchr(argv[i + 1], '='))
          mlt_properties_parse(new_props, argv[++i]);
      }
    }
  } else
    for (i = 1; i < argc; i++) {
      if (!strcmp(argv[i], "-consumer")) {
        if (*consumer) mlt_consumer_close(*consumer);
        *consumer = create_consumer(profile, argv[++i]);
        if (*consumer) {
          mlt_properties properties = MLT_CONSUMER_PROPERTIES(*consumer);
          while (argv[i + 1] != NULL && strchr(argv[i + 1], '='))
            mlt_properties_parse(properties, argv[++i]);
        }
      }
    }
}

static void on_consumer_stopped(mlt_properties owner, mlt_consumer consumer,
                                mlt_event_data event_data) {
  // TODO
  printf("on_consumer_stopped\n");
  mlt_instance instance = mlt_properties_get_data(
      MLT_CONSUMER_PROPERTIES(consumer), "mlt_instance", 0);
  if (instance != NULL) {
    instance->state = STATE_STOPPED;
  }
}

static void on_consumer_fatal_error(mlt_properties owner, mlt_consumer consumer,
                                    mlt_event_data event_data) {
  // TODO
  printf("on_consumer_fatal_error\n");
  mlt_properties_set_int(MLT_CONSUMER_PROPERTIES(consumer), "done", 1);
  mlt_properties_set_int(MLT_CONSUMER_PROPERTIES(consumer), "melt_error", 1);
  mlt_instance instance = mlt_properties_get_data(
      MLT_CONSUMER_PROPERTIES(consumer), "mlt_instance", 0);
  if (instance != NULL) {
    instance->state = STATE_STOPPED;
    instance->error = ERROR_FATAL;
  }
}

static void set_preview_scale(mlt_profile profile, double scale) {
  profile->width *= scale;
  profile->width -= profile->width % 2;
  profile->height *= scale;
  profile->height -= profile->height % 2;
}

static bool is_profile_equals(mlt_profile a, mlt_profile b) {
  return a->width == b->width && a->height == b->height &&
         a->sample_aspect_num == b->sample_aspect_num &&
         a->sample_aspect_den == b->sample_aspect_den &&
         a->frame_rate_den == b->frame_rate_den &&
         a->frame_rate_num == b->frame_rate_num &&
         a->colorspace == b->colorspace;
}

/**
 * TODO 支持传入 producer consumer 在 argv 或 xml 里用占位表达
 */
EMSCRIPTEN_KEEPALIVE mlt_instance mlt_instance_create(int argc, char** argv) {
  mlt_instance instance = calloc(1, sizeof(struct mlt_instance_s));
  instance->argc = argc;
  // TODO 是否需要拷贝 argv
  instance->argv = argv;
  pthread_mutex_init(&instance->mutex, NULL);
  return instance;
}

/**
 * \return 非 0 代表失败
 */
EMSCRIPTEN_KEEPALIVE int mlt_instance_init(mlt_instance instance) {
  int argc = instance->argc;
  char** argv = instance->argv;
  mlt_profile profile = NULL;
  mlt_profile backup_profile = NULL;
  mlt_producer melt = NULL;
  mlt_consumer consumer = NULL;
  const char* profile_name = NULL;
  bool is_consumer_explicit = true;
  bool success = false;

  if (argc <= 1) goto exit_create;

  if (mlt_factory_repository() == NULL) mlt_factory_init(NULL);

  // TODO
  printf("argc:%d\n", argc);

  for (int i = 1; i < argc; i++) {
    printf("argv:%d %s\n", i, argv[i]);
    // Look for the profile option
    if (!strcmp(argv[i], "-profile")) {
      const char* pname = argv[++i];
      if (pname && pname[0] != '-') profile_name = pname;
    }
  }

  // Create profile if not set explicitly
  // TODO 默认 profile
  profile = mlt_profile_init(profile_name);

  // Look for the consumer option to load profile settings from consumer
  // properties
  backup_profile = mlt_profile_clone(profile);
  load_consumer(&consumer, profile, argc, argv);
  bool need_reload_consumer = false;

  // If the consumer changed the profile, then it is explicit.
  if (backup_profile && !profile->is_explicit &&
      !is_profile_equals(profile, backup_profile))
    profile->is_explicit = 1;
  mlt_profile_close(backup_profile);
  backup_profile = NULL;

  // Get melt producer
  melt = mlt_factory_producer(profile, "melt", &argv[1]);

  if (melt == NULL) goto exit_create;

  // Generate an automatic profile if needed.
  if (!profile->is_explicit) {
    backup_profile = mlt_profile_clone(profile);
    // mlt_profile_from_producer 会调用 mlt_service_get_frame 可能存在主线程阻塞
    mlt_profile_from_producer(profile, melt);
    if (!is_profile_equals(profile, backup_profile)) {
      // TODO
      printf("##### recreate melt old_profile: %s; new_profile: %s :%dx%d\n",
             backup_profile->description, profile->description, profile->width,
             profile->height);
      mlt_producer_close(melt);
      melt = mlt_factory_producer(profile, "melt", &argv[1]);
      mlt_profile_close(backup_profile);
      backup_profile = NULL;
      if (melt == NULL) goto exit_create;
      need_reload_consumer = true;
    }
  }

  if (consumer != NULL) {
    double scale =
        mlt_properties_get_double(MLT_CONSUMER_PROPERTIES(consumer), "scale");
    if (scale > 0.0) {
      set_preview_scale(profile, scale);
      need_reload_consumer = true;
    }

    // Reload the consumer with the fully qualified profile.
    // The producer or auto-profile could have changed the profile.
    if (need_reload_consumer) {
      // TODO
      printf("##### reload consumer\n");
      load_consumer(&consumer, profile, argc, argv);
    }
  }

  // See if producer has consumer already attached
  if (consumer == NULL) {
    consumer = MLT_CONSUMER(mlt_service_consumer(MLT_PRODUCER_SERVICE(melt)));
    if (consumer != NULL) {
      mlt_properties_inc_ref(
          MLT_CONSUMER_PROPERTIES(consumer));  // because we explicitly close it
    }
    is_consumer_explicit = false;
  }

// If we have no consumer, default to sdl
#if (!defined(__EMSCRIPTEN__))
  if (consumer == NULL) consumer = create_consumer(profile, NULL);
#endif

  if (consumer == NULL) goto exit_create;

  success = true;

exit_create:

  if (success) {
    instance->producer = melt;
    instance->consumer = consumer;
    instance->profile = profile;
    instance->is_consumer_explicit = is_consumer_explicit;
    instance->state = STATE_INITIALIZED;
    instance->error = 0;
    mlt_properties_set_data(MLT_CONSUMER_PROPERTIES(consumer), "mlt_instance",
                            instance, 0, NULL, NULL);
    return 0;
  } else {
    if (consumer != NULL) {
      mlt_consumer_close(consumer);
    }

    if (melt != NULL) mlt_producer_close(melt);

    mlt_profile_close(profile);
    instance->state = STATE_CREATED;
    instance->error = ERROR_INIT;
    return 1;
  }
}

/**
 * Start the consumer.
 *
 * \return 非 0 代表失败
 */
EMSCRIPTEN_KEEPALIVE int mlt_instance_start(mlt_instance instance) {
  mlt_producer melt = instance->producer;
  mlt_consumer consumer = instance->consumer;
  bool is_consumer_explicit = instance->is_consumer_explicit;

  if (melt == NULL || consumer == NULL) {
    instance->error = ERROR_START;
    return 1;
  }

  if (!mlt_consumer_is_stopped(consumer)) return 1;

  mlt_properties_set_data(MLT_CONSUMER_PROPERTIES(consumer),
                          "transport_producer", melt, 0, NULL, NULL);
  mlt_properties_set_data(MLT_PRODUCER_PROPERTIES(melt), "transport_consumer",
                          consumer, 0, NULL, NULL);

  if (mlt_producer_get_length(melt) <= 0) return 0;

  // Get melt's properties
  mlt_properties melt_props = MLT_PRODUCER_PROPERTIES(melt);
  mlt_properties consumer_props = MLT_CONSUMER_PROPERTIES(consumer);

  if (is_consumer_explicit) {
    // Apply group settings
    mlt_properties group = mlt_properties_get_data(melt_props, "group", 0);
    mlt_properties_inherit(consumer_props, group);
  }

  int in = mlt_properties_get_int(consumer_props, "in");
  int out = mlt_properties_get_int(consumer_props, "out");
  if (in > 0 || out > 0) {
    if (out == 0) {
      out = mlt_producer_get_length(melt) - 1;
    }
    mlt_producer_set_in_and_out(melt, in, out);
    mlt_producer_seek(melt, 0);
  }

  // Connect consumer to melt
  mlt_consumer_connect(consumer, MLT_PRODUCER_SERVICE(melt));

  mlt_events_listen(consumer_props, consumer, "consumer-stopped",
                    (mlt_listener)on_consumer_stopped);
  mlt_events_listen(consumer_props, consumer, "consumer-fatal-error",
                    (mlt_listener)on_consumer_fatal_error);

  mlt_properties_set_int(MLT_CONSUMER_PROPERTIES(consumer), "melt_error", 0);
  instance->error = 0;
  instance->state = STATE_RUNNING;
  int error = mlt_consumer_start(consumer);
  if (mlt_consumer_is_stopped(consumer)) {
    // consumer 是同步执行的或启动失败了
    instance->state = STATE_STOPPED;
  }
  if (error) {
    instance->error = ERROR_START;
  }
  return error;
}

static void* init_and_start_thread(void* param) {
  mlt_instance instance = param;
  pthread_mutex_lock(&instance->mutex);
  if (instance->is_starting) {
    if (mlt_instance_init(instance) == 0) {
      mlt_instance_start(instance);
    }
  }
  instance->is_starting = false;
  pthread_mutex_unlock(&instance->mutex);
  return NULL;
}

EMSCRIPTEN_KEEPALIVE void mlt_instance_start_with_init(mlt_instance instance) {
  if (instance->state == STATE_CREATED) {
    instance->state = STATE_INITIALIZING;
    instance->is_starting = true;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t thread;
    pthread_create(&thread, &attr, init_and_start_thread, instance);
    pthread_attr_destroy(&attr);
  } else if (!instance->is_starting) {
    mlt_instance_start(instance);
  }
}

EMSCRIPTEN_KEEPALIVE void mlt_instance_stop(mlt_instance instance) {
  bool need_lock = false;
  if (instance->is_starting) {
    need_lock = true;
    pthread_mutex_lock(&instance->mutex);
    instance->is_starting = false;
  }
  mlt_consumer consumer = instance->consumer;
  if (consumer != NULL) {
    if (!mlt_consumer_is_stopped(consumer)) mlt_consumer_stop(consumer);
    instance->state = STATE_STOPPED;
  }

  if (need_lock) pthread_mutex_unlock(&instance->mutex);
}

EMSCRIPTEN_KEEPALIVE int mlt_instance_is_starting(mlt_instance instance) {
  return instance->is_starting;
}

EMSCRIPTEN_KEEPALIVE int mlt_instance_is_running(mlt_instance instance) {
  mlt_consumer consumer = instance->consumer;
  if (consumer == NULL) return 0;
  return !mlt_consumer_is_stopped(consumer);
}

EMSCRIPTEN_KEEPALIVE int mlt_instance_is_stopped(mlt_instance instance) {
  if (instance->is_starting) return 0;
  mlt_consumer consumer = instance->consumer;
  if (consumer == NULL) return 1;
  return mlt_consumer_is_stopped(consumer);
}

EMSCRIPTEN_KEEPALIVE double mlt_instance_get_progress(mlt_instance instance) {
  mlt_producer producer = instance->producer;
  if (producer == NULL) return 0;

  int total_length = mlt_producer_get_length(producer);
  int current_position = mlt_producer_position(producer);
  if (total_length <= 0) {
    return 0;
  }
  return current_position / (double)total_length;
}

/**
 * \return 非 0 代表错误代码
 */
EMSCRIPTEN_KEEPALIVE int mlt_instance_get_error(mlt_instance instance) {
  return instance->error;
}

EMSCRIPTEN_KEEPALIVE void mlt_instance_release(mlt_instance instance,
                                               bool release_argv) {
  if (instance->state == STATE_RELEASED) return;
  bool need_lock = false;
  if (instance->is_starting) {
    need_lock = true;
    pthread_mutex_lock(&instance->mutex);
    instance->is_starting = false;
  }
  mlt_producer melt = instance->producer;
  mlt_consumer consumer = instance->consumer;
  mlt_profile profile = instance->profile;

  if (consumer != NULL) {
    if (!mlt_consumer_is_stopped(consumer)) mlt_consumer_stop(consumer);
    // Disconnect producer from consumer to prevent ref cycles from closing
    // services
    mlt_consumer_connect(consumer, NULL);
    mlt_consumer_close(consumer);
  }

  if (melt != NULL) mlt_producer_close(melt);

  if (need_lock) pthread_mutex_unlock(&instance->mutex);

  mlt_profile_close(profile);

  int argc = instance->argc;
  char** argv = instance->argv;
  if (argv != NULL && release_argv) {
    for (int i = 0; i < argc; ++i) {
      free(argv[i]);
    }
    free(argv);
  }
  pthread_mutex_destroy(&instance->mutex);

  instance->producer = NULL;
  instance->consumer = NULL;
  instance->profile = NULL;
  instance->argv = NULL;
  instance->state = STATE_RELEASED;

  free(instance);
}

int main(int argc, char** argv) {
  mlt_instance instance = mlt_instance_create(argc, argv);

  if (instance == NULL) {
    printf("mlt_instance_create return NULL\n");
    return 1;
  }

  //   if (mlt_instance_init(instance) != 0) {
  //     printf("mlt_instance_init error\n");
  //     return 1;
  //   }
  //   mlt_instance_start(instance);

  mlt_instance_start_with_init(instance);

  //   mlt_instance_stop(instance);
  //   printf("mlt_instance_is_stopped: %d\n",
  //   mlt_instance_is_stopped(instance));

  // 40ms
  struct timespec tm = {0, 40000000};
  while (!mlt_instance_is_stopped(instance)) {
    nanosleep(&tm, NULL);
    printf("progress: %f\n", mlt_instance_get_progress(instance));
  }
  printf("progress: %f\n", mlt_instance_get_progress(instance));

  int error = mlt_instance_get_error(instance);

  printf("finish error: %d\n", error);

  // instance 重复使用
  //   mlt_instance_start(instance);
  //   while (!mlt_instance_is_stopped(instance)) {
  //     nanosleep(&tm, NULL);
  //   }
  //   error = mlt_instance_get_error(instance);
  //   printf("finish2 error: %d\n", error);

  mlt_instance_release(instance, false);

  return error;
}
