# MagCam

## Introduction
MagCam is a project that aims to bring a better way of teaching principles of magnetism in a classroom environment. Using this device we hope that students will be more engaged in learning about various topics regarding magnetism as well as electromagnetism. During the process of making this device we looked for a way to improve on the traditional methods of displaying magnetic fields.  

## Comparison with traditional methods

|  Method | Presentation | Versatility | Sensitivity | Polarity | Extra Notes |
|---------|--------------|-------------|-------------|----------| ----------- |
| MagCam  | The results can be displayed on a projector | Very versatile in terms of experiments that can be performed | Sensitive to small magnetic fields but limited by the ADC and pre-amp | Has the ability to distinguish between **north** and **south** poles | Can record the data captured
| Iron Dust | Limited number of students can see the results at one time | Number of experiments that can be performed is limited | Only sensitive to strong magnetic fields. Better results can be achieved if particles are suspended in liquid | Doesn't have a way to clearly distinguish between 2 poles | Can stick to the magnets and damage other equipment in the lab |  
| [Flux Detector](https://www.supermagnete.de/eng/school-magnets/flux-detector-small_M-04) | Limited number of students can see the results at one time | Number of experiments that can be performed is sufficient for basic needs | Only sensitive to strong magnetic fields | Doesn't have a way to clearly distinguish between 2 poles |
| Compass array | Limited number of students can see the results at one time |  Very versatile in terms of experiments that can be performed | Sensitive to weak magnetic fields | Has the ability to distinguish between **north** and **south** poles | The device is composed of mechanical compasses and thus needs to be kept horizontal |
| Ferro fluid | Limited number of students can see the results at one time | Number of experiments that can be performed is sufficient for basic needs | Only sensitive to strong magnetic fields | Doesn't have a way to clearly distinguish between 2 poles | Has the possibility to damage other equipment and is hard to clean |
| Virtual demonstration | The results can be displayed on a projector | Any experiment that can be virtually modeled can be performed | The most sensitive option | Has the ability to distinguish between **north** and **south** poles | Requires special training in use of simulation software. It can take a long time for results to be ready. This method doesn't measure magnetic fields and only **simulates** them |

## Basic overview 
MagCam uses 256 hall effect sensors to determine the strength of the magnetic field over a detection area of 110x110 mm. This data is read and sent to the computer for further processing by and esp32 microcontroller. The data is processed and displayed on the computer using an app that was written specifically for this device.

## App
The app reads the values sent by the esp32 and decodes them into individual frames. This data is then calibrated using a baseline value for each sensors. This improves the reading accuracy and allows for less noisy results. In case large gain is used the values received from the device can be averaged. This allows for even less noisy results. 
The app also has a feature that recreates the magnetic field lines from the change in local magnet field strength.
> App is still in development  and more features are planned in the future. There are also plans to improve the gui

## Files
all the files can be found [here](https://github.com/miki407/MagCam)
