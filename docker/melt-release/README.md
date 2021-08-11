构建
```
docker build -t my-melt:1.0 -f docker/melt-release/Dockerfile .
```

运行
```
docker run -v path/to/vol:/vol my-melt:1.0 /vol/xxx.mp4 -consumer avformat:/vol/output.mp4
```
 