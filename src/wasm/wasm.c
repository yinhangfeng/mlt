// #ifndef _GNU_SOURCE
// #define _GNU_SOURCE
// #endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>

#include <framework/mlt.h>

#if (defined(__APPLE__) || defined(_WIN32) || defined(HAVE_SDL2)) && !defined(MELT_NOSDL)
#include <SDL.h>
#endif

// TODO 命名
struct mlt_instance_s
{
    mlt_producer producer;
    mlt_consumer consumer;
    mlt_profile profile;
}

typedef struct mlt_instance_s *mlt_instance;

static mlt_consumer create_consumer(mlt_profile profile, char *id)
{
    char *myid = id ? strdup(id) : NULL;
    char *arg = myid ? strchr(myid, ':') : NULL;
    if (arg != NULL)
        *arg++ = '\0';
    mlt_consumer consumer = mlt_factory_consumer(profile, myid, arg);
    if (consumer != NULL)
    {
        mlt_properties properties = MLT_CONSUMER_PROPERTIES(consumer);
        // mlt_properties_set_data( properties, "transport_callback", transport_action, 0, NULL, NULL );
    }
    free(myid);
    return consumer;
}

static void load_consumer(mlt_consumer *consumer, mlt_profile profile, int argc, char **argv)
{
    int i;
    int multi = 0;
    int qglsl = 0;

    for (i = 1; i < argc; i++)
    {
        // See if we need multi consumer.
        multi += !strcmp(argv[i], "-consumer");
        // Seee if we need the qglsl variant of multi consumer.
        if (!strncmp(argv[i], "glsl.", 5) || !strncmp(argv[i], "movit.", 6))
            qglsl = 1;
    }
    // Disable qglsl if xgl is being used!
    for (i = 1; qglsl && i < argc; i++)
        if (!strcmp(argv[i], "xgl"))
            qglsl = 0;

    if (multi > 1 || qglsl)
    {
        // If there is more than one -consumer use the 'multi' consumer.
        int k = 0;
        char key[20];

        if (*consumer)
            mlt_consumer_close(*consumer);
        *consumer = create_consumer(profile, (qglsl ? "qglsl" : "multi"));
        mlt_properties properties = MLT_CONSUMER_PROPERTIES(*consumer);
        for (i = 1; i < argc; i++)
        {
            if (!strcmp(argv[i], "-consumer") && argv[i + 1])
            {
                // Create a properties object for each sub-consumer
                mlt_properties new_props = mlt_properties_new();
                snprintf(key, sizeof(key), "%d", k++);
                mlt_properties_set_data(properties, key, new_props, 0,
                                        (mlt_destructor)mlt_properties_close, NULL);
                if (strchr(argv[i + 1], ':'))
                {
                    char *temp = strdup(argv[++i]);
                    char *service = temp;
                    char *target = strchr(temp, ':');
                    *target++ = 0;
                    mlt_properties_set(new_props, "mlt_service", service);
                    mlt_properties_set(new_props, "target", target);
                }
                else
                {
                    mlt_properties_set(new_props, "mlt_service", argv[++i]);
                }
                while (argv[i + 1] && strchr(argv[i + 1], '='))
                    mlt_properties_parse(new_props, argv[++i]);
            }
        }
    }
    else
        for (i = 1; i < argc; i++)
        {
            if (!strcmp(argv[i], "-consumer"))
            {
                if (*consumer)
                    mlt_consumer_close(*consumer);
                *consumer = create_consumer(profile, argv[++i]);
                if (*consumer)
                {
                    mlt_properties properties = MLT_CONSUMER_PROPERTIES(*consumer);
                    while (argv[i + 1] != NULL && strchr(argv[i + 1], '='))
                        mlt_properties_parse(properties, argv[++i]);
                }
            }
        }
}

static void transport(mlt_producer producer, mlt_consumer consumer)
{
    mlt_properties properties = MLT_PRODUCER_PROPERTIES(producer);
    struct timespec tm = {0, 40000000};
    // TODO
    while (!mlt_consumer_is_stopped(consumer) && mlt_properties_get_int(properties, "done") == 0)
    {
        nanosleep(&tm, NULL);
    }
}

static void on_fatal_error(mlt_properties owner, mlt_consumer consumer, mlt_event_data event_data)
{
    mlt_properties_set_int(MLT_CONSUMER_PROPERTIES(consumer), "done", 1);
    mlt_properties_set_int(MLT_CONSUMER_PROPERTIES(consumer), "melt_error", 1);
}

static void set_preview_scale(mlt_profile *profile, mlt_profile *backup_profile, double scale)
{
    *backup_profile = mlt_profile_clone(*profile);
    if (*backup_profile)
    {
        mlt_profile temp = *profile;
        *profile = *backup_profile;
        *backup_profile = temp;
        (*profile)->width *= scale;
        (*profile)->width -= (*profile)->width % 2;
        (*profile)->height *= scale;
        (*profile)->height -= (*profile)->height % 2;
    }
}

// TODO 支持传入 producer consumer 在 argv 或 xml 里用占位表达
// argv 需要由外部释放
mlt_instance mlt_instance_create(int argc, char **argv)
{
    mlt_profile profile = NULL;
    mlt_profile backup_profile = NULL;
    mlt_producer melt = NULL;
    mlt_consumer consumer = NULL;
    int is_consumer_explicit = 0;

    if (!mlt_factory_repository())
        mlt_factory_init(NULL);

    // TODO
    printf("argc:%d\n", argc);

    for (int i = 1; i < argc; i++)
    {
        printf("argv:%d %s\n", i, argv[i]);
        // Look for the profile option
        if (!strcmp(argv[i], "-profile"))
        {
            const char *pname = argv[++i];
            if (pname && pname[0] != '-')
                profile = mlt_profile_init(pname);
        }
        else if (!strcmp(argv[i], "-consumer"))
        {
            is_consumer_explicit = 1;
        }
    }

    // 必须指定 consumer TODO
    // if (!is_consumer_explicit)
    //     return NULL;

    // Create profile if not set explicitly
    // TODO 默认 profile
    if (profile == NULL)
        profile = mlt_profile_init(NULL);

    // Look for the consumer option to load profile settings from consumer properties
    backup_profile = mlt_profile_clone(profile);
    load_consumer(&consumer, profile, argc, argv);

    // If the consumer changed the profile, then it is explicit.
    if (backup_profile && !profile->is_explicit && (profile->width != backup_profile->width || profile->height != backup_profile->height || profile->sample_aspect_num != backup_profile->sample_aspect_num || profile->sample_aspect_den != backup_profile->sample_aspect_den || profile->frame_rate_den != backup_profile->frame_rate_den || profile->frame_rate_num != backup_profile->frame_rate_num || profile->colorspace != backup_profile->colorspace))
        profile->is_explicit = 1;
    mlt_profile_close(backup_profile);
    backup_profile = NULL;

    // Get melt producer
    if (argc > 1)
        melt = mlt_factory_producer(profile, "melt", &argv[1]);

    if (melt)
    {
        // Generate an automatic profile if needed.
        if (!profile->is_explicit)
        {
            mlt_profile_from_producer(profile, melt);
            // TODO 为什么要重新创建?
            mlt_producer_close(melt);
            melt = mlt_factory_producer(profile, "melt", &argv[1]);
        }

        double scale = mlt_properties_get_double(MLT_CONSUMER_PROPERTIES(consumer), "scale");
        if (scale > 0.0)
        {
            set_preview_scale(&profile, &backup_profile, scale);
        }

        // Reload the consumer with the fully qualified profile.
        // The producer or auto-profile could have changed the profile.
        // TODO 是否可以避免重复创建
        load_consumer(&consumer, profile, argc, argv);

        // See if producer has consumer already attached
        if (!consumer)
        {
            consumer = MLT_CONSUMER(mlt_service_consumer(MLT_PRODUCER_SERVICE(melt)));
            if (consumer)
            {
                mlt_properties_inc_ref(MLT_CONSUMER_PROPERTIES(consumer)); // because we explicitly close it
                // mlt_properties_set_data( MLT_CONSUMER_PROPERTIES(consumer),
                // 	"transport_callback", transport_action, 0, NULL, NULL );
            }
        }

        // If we have no consumer, default to sdl
        // if ( consumer == NULL )
        // 	consumer = create_consumer( profile, NULL );
    }

    mlt_profile_close(backup_profile);

    mlt_instance = malloc(sizeof(struct mlt_instance_s));
    if (mlt_instance)
    {
        mlt_instance->producer = melt;
        mlt_instance->consumer = consumer;
    }

    return mlt_instance;
}

// TODO 异步结束回调
void mlt_instance_start(mlt_instance instance)
{
    mlt_producer melt = instance->producer;
    mlt_consumer consumer = instance->consumer;

    if (melt == NULL || consumer == NULL)
        return;

    mlt_properties_set_data(MLT_CONSUMER_PROPERTIES(consumer), "transport_producer", melt, 0, NULL, NULL);
    mlt_properties_set_data(MLT_PRODUCER_PROPERTIES(melt), "transport_consumer", consumer, 0, NULL, NULL);

    if (mlt_producer_get_length(melt) > 0)
    {
        // Get melt's properties
        mlt_properties melt_props = MLT_PRODUCER_PROPERTIES(melt);
        mlt_properties consumer_props = MLT_CONSUMER_PROPERTIES(consumer);

        // if (is_consumer_explicit)
        // {
        // melt 的 group 参数
        // Apply group settings
        mlt_properties group = mlt_properties_get_data(melt_props, "group", 0);
        mlt_properties_inherit(consumer_props, group);
        // }

        int in = mlt_properties_get_int(consumer_props, "in");
        int out = mlt_properties_get_int(consumer_props, "out");
        if (in > 0 || out > 0)
        {
            if (out == 0)
            {
                out = mlt_producer_get_length(melt) - 1;
            }
            mlt_producer_set_in_and_out(melt, in, out);
            mlt_producer_seek(melt, 0);
        }

        // Connect consumer to melt
        mlt_consumer_connect(consumer, MLT_PRODUCER_SERVICE(melt));

        // Start the consumer
        mlt_events_listen(consumer_props, consumer, "consumer-fatal-error", (mlt_listener)on_fatal_error);
        if (mlt_consumer_start(consumer) == 0)
        {
            // Transport functionality
            transport(melt, consumer);

            // Stop the consumer
            mlt_consumer_stop(consumer);
        }
    }

    // TODO 获取 consumer 是否报错
    int error = mlt_properties_get_int(MLT_CONSUMER_PROPERTIES(consumer), "melt_error");
}

void mlt_instance_stop(mlt_instance instance)
{
    mlt_consumer consumer = instance->consumer;
    if (!mlt_consumer_is_stopped(consumer))
        mlt_consumer_stop(consumer);
}

int mlt_instance_is_stopped(mlt_instance instance)
{
    return mlt_consumer_is_stopped(instance->consumer);
}

void mlt_instance_release(mlt_instance instance)
{
    mlt_producer melt = instance->producer;
    mlt_consumer consumer = instance->consumer;
    mlt_profile profile = instance->profile;

    if (consumer != NULL)
    {
        // Disconnect producer from consumer to prevent ref cycles from closing services
        mlt_consumer_connect(consumer, NULL);
        mlt_consumer_close(consumer);
    }

    if (melt != NULL)
        mlt_producer_close(melt);

    mlt_profile_close(profile);

    free(instance);
}

int main(int argc, char **argv)
{
    mlt_instance instance = mlt_instance_create(argc, argv);

    if (instance == NULL)
    {
        printf("mlt_instance_create error");
        return 1;
    }

    mlt_instance_start(instance);

    mlt_instance_release(instance);
    return 0;
}
