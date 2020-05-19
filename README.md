# Turn a .rec file with WGS84 coordinates into a .png file with a GPS trace

```
# Have a file like myRecording.rec in the current folder, then:
docker run --rm -ti -v $PWD:/data -w /data chrberger/wgs84topng:v0.0.1 myRecording.rec myRecording.png
