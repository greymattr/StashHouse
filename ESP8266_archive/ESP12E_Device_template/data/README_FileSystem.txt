Place files to be uploaded to SPIFFS/TinyFS file system in this data folder and upload
them with the command:
# pio run -t uploadfs

add below to platformio.ini to specify FS type
board_build.filesystem = littlefs
