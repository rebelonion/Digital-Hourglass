
# Digital Hourglass

Sick of using a regular old sand timer for those board game nights? Then this project is for you! No more waiting for the sand to finish falling for the next round. No more fights about the exact millisecond the last drop of sand fell, and if that last point counted or not. This digital sand timer features an instant reset, flashing lights, a buzzer, and so much more!

This started out as a college project that I went a little overboard for. It turned into a really fun experience of developing a project from scratch. This is also my first (completed) project that uses SMT PCBs.

Also, **This is very much still a awork in progress**

<img src="https://s11.gifyu.com/images/ezgif-3-dc91cbdd8c.gif" width="200" height="400">


## Features

- Rechargeable (18650 cell)
- Time can be set
- Time resets on flip
- RGB LEDs change from green to red as time runs out
- Buzzer

## Known Problems

- Accelerometer can take a few seconds to wake from cold start
- LEDs draw ~100mA while off (common for WS2812)
- Device cannot turn itself off
- Current buzzer is quiet

## PCBs
|||
|:-------------:|:-------------:|
|Main PCB|[OSHWLab link](https://oshwlab.com/somdahlfinnley/timer-hourglass)|
|||
|LED PCB|[OSHWLab link](https://oshwlab.com/somdahlfinnley/hourglass_leds)|
|||
|7-Segment PCB|[OSHWLab link](https://oshwlab.com/somdahlfinnley/hourglass_7seg)|
|||

## 3D Print Files
A .step file and a .f3d file are included in the 3D Files directory. There are 4 separate objects that need to be printed.

## Uploading Code
For this project, I used PlatformIO with the Attiny3226. code is uploaded with [jtag2udpi](https://github.com/ElTangas/jtag2updi)

## Datasheets 
|||
|:-:|:-:|
|Attiny3226|[Link](https://www.mouser.com/datasheet/2/268/ATtiny3224_3226_3227_Data_Sheet_DS40002345B-2940847.pdf)|
|||
|LIS3DH|[Link](https://www.mouser.com/datasheet/2/389/lis3dh-1849589.pdf)|
|||
|AS1115|[Link](https://www.mouser.com/datasheet/2/588/AS1115_DS000206_1_00-1512924.pdf)|
|||
|MP2632|[Link](https://www.monolithicpower.com/en/documentview/productdocument/index/version/2/document_type/Datasheet/lang/en/sku/MP2632/document_id/1446/)|
|||

## Pictures

| <img src="https://i.ibb.co/NykYhf7/20230427-194008.jpg" data-canonical-src="https://i.ibb.co/NykYhf7/20230427-194008.jpg" width="300" height="600" />        | <img src="https://i.ibb.co/q1pr0GS/20230427-193904.jpg" data-canonical-src="https://i.ibb.co/q1pr0GS/20230427-193904.jpg" width="300" height="600" />           |
| ------------- |:-------------:|


