实现一个翻页器

- 可能利用 ESP32
- 至少支持两个以上的按键输入
- 支持充放电（包含电池）
- 页面可配置按键的具体输入

从 Arduino 转换到 PlatformIO August 7, 2022 8:53 PM

- 卡在 Rebuilding IntelliSense Index，暂时还是使用 arduino ide，再看好像又没问题了。
- Arduino ide 会莫名其妙占用非常多的内存和CPU资源。 August 11, 2022 9:46 AM
- 成功导入 esp32-ble-keyboard 库，成功完成代码迁移，完成编译和上传测试。 August 11, 2022 9:54 AM
- 尝试配置写入分区设置的时候，出现错误 lld_pdu_get_tx_flush_nb HCI packet count mismatch (0, 1)。这个错误与分区表问题无关，忘记写入后要连接设备了。
- 使用内置分区表 default_8MB.csv 的时候会不断重启，可能需要进一步测试分区表设置。
    - 修改 board 为某一个8MB的开发板就能成功。
    - 增加配置 board_upload.flash_size = 8MB ，无需修改默认开发板配置即可成功。
    - https://github.com/espressif/arduino-esp32/issues/6646