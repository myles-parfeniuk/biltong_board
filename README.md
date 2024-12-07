<a name="readme-top"></a>

![image](documentation/README_IMAGES/banner.png)

# About
Sun drying emulation controller for biltong and other dried foodstuffs,  based around STM32G070 series controller.  
Intended to be used with drying enclosures like biltong-boxes to produce sun-dried foods in non-ideal climates. 

# Table of Contents

- [About](#about)
- [PCBWay Sponsorship](#pcbway-sponsorship)
- [Hardware](#hardware)
  - [System Block Diagram](#system-block-diagram)
  - [PCB Renders](#pcb-renders)
  - [Layout](#layout)
  - [Schematics](#schematics)
- [Firmware](#firmware)
- [License](#license)
- [Contact](#contact)


# PCBWay Sponsorship
Thank you to PCBWay for reaching out to sponsor this project and provide access to their excellent service.  
PCBWay supports the DIY and open source community as well as students with sponsorships and contests frequently.   
Show them some love and consider having your next project fabricated by them: 


![image](documentation/README_IMAGES/PCB_way_logo.png) 

Order [here](https://www.pcbway.com/) today.

This was my second time using PCBWay's service, and just like last time the boards came out perfect with no defects.
While assembling the boards at home with a friend, I even had him comment the silk screen and cosmetic qualities of the board were
superior to that of domestic fabrication houses he has seen used throughout his professional career.   
To be clear, we are talking about a 0.8mm character height, with no smudging or defected labels on any part of the board, amazing!

The manufacturing and shipping time was just as fast as last time, under 2 weeks from gerber files to packages on my doorstep. 
Their customer service was also top notch. The packages were initially thought to be lost by the postage service, they offered to replace them at no cost; but, thankfully they were found the next day.  

Working with PCBWay both on this project and my previous project [here](https://github.com/myles-parfeniuk/wsen_hids_breakout) has made them my goto fabrication house. The cost to quality ratio ratio provided by their service is by far the best I have encountered. It would take a serious competitor to make me think otherwise. 

Below are some photos of the assembled main boards, along with a 3D printed case to protect the HV parts of the board, designed by [hwBirdy007](https://github.com/hwBirdy007). The quality I'm speaking of is evident when viewing, next time I want to try a different color solder mask, as PCBWay provides a wide variety of choices at no extra cost (green ,red, yellow, blue, white , and black).  
Note that the bodge wires on Y2 (low speed oscillator for RTC) are not the fault of PCBWay, I used an incorrect schematic symbol leading to an erroneous design, oops. Looks like I'll be using PCBWay again soon. This error has already been corrected in the provided KiCad files and generated gerbers.

![image](documentation/README_IMAGES/board_real_front.jpg) 
![image](documentation/README_IMAGES/board_real_back.jpg) 
![image](documentation/README_IMAGES/board_real_case_front.jpg) 
![image](documentation/README_IMAGES/board_real_case_back.jpg) 
<p align="right">(<a href="#readme-top">back to top</a>)</p>

# Hardware

Biltong board is composed of 4 separate PCBs: a main driver board, a UI/control deck board, and 2 identical temperature + humidity sensor breakout boards.   

The schematics and layout files for all boards can be found within the `hardware` directory of this repo, in the form of KiCad 7.0 projects.

The key features of each board:

Main board:
- Interfaces to drive sensor breakout boards, the control deck board,  heat lamps, and +12V PC fans.
- Stand alone power supply, no DC adapter required. 
- A magnetically isolated current sensor for linearizing the response of the triac based heat-lamp dimmer circuit. 
- A low speed oscillator and coin-cell battery interfaced to STM32's RTC for time keeping during power loss.
- A normally open safety relay to remove the lamps from circuit.
- IO expansion header, with unused GPIO for development of new features or debugging. 
- Extra mounting holes for safety cover, to prevent mishaps with high voltage side of board. 

UI/control deck board: 
- An SPI based SH1122 driven OLED display for indicating information to the user.
- 3 navigation buttons for user input.
- A buzzer to notify user attention is required. 

Sensor Breakout Board: 
- I2C based [temperature and humidity sensor](https://www.we-online.com/en/components/products/WSEN-HIDS) from Würth Elektronik for closing loop between fans and lamps.

## System Block Diagram
![image](documentation/README_IMAGES/physical_block_diagram.png)

| SIGNAL       | PIN # | GPIO# | PERIPHERAL                       | DESCRIPTION                                                                                 |
|--------------|-------|-------|----------------------------------|---------------------------------------------------------------------------------------------|
| DISP_SCL     | 12    | PA1   | SPI1                             | SH1122 display SPI clock signal.                                                            |
| I2C1_SDA     | 32    | PA10  | I2C1                             | Temp & humidity sensor  A I2C data signal.                                                  |
| I2C2_SCL     | 33    | PA11  | I2C2                             | Temp & humidity sensor A I2C clock signal.                                                  |
| I2C2_SDA     | 34    | PA12  | I2C2                             | Temp & humidity sensor  B I2C data signal.                                                  |
| DISP_MOSI    | 13    | PA2   | SPI1                             | SH1122 display SPI data signal.                                                             |
| SW_UP        | 14    | PA3   | GPIO                             | Active low user up navigation button signal.                                                           |
| SW_ENTER     | 15    | PA4   | GPIO                             | Active low user enter/select navigation button signal.                                                 |
| SW_DOWN      | 16    | PA5   | GPIO                             | Active low user down navigation button signal.                                                         |
| FAN_B_PWM    | 17    | PA6   | TIM16_CH1 or TIM3_CH1            | Fan B PWM signal.                                                                           |
| FAN_B_TACH   | 18    | PA7   | GPIO or  TIM17_CH1 or  TIM14_CH1 | Fan B tachometer signal.                                                                    |
| I2C1_SCL     | 29    | PA9   | I2C1                             | Temp & humidity sensor B I2C clock signal.                                                  |
| FAN_A_TACH   | 19    | PB0   | GPIO or  TIM3_CH3                | Fan A tachometer signal.                                                                    |
| FAN_A_PWM    | 20    | PB1   | TIM14_CH1                        | Fan A PWM signal.                                                                           |
| UART_TX      | 22    | PB10  | USART3                           | Serial console UART TX signal for development.                                              |
| UART_RX      | 23    | PB11  | USART3                           | Serial console UART RX signal  for development.                                             |
| I_SENSE      | 21    | PB2  | ADC_IN10                         | Heat lamp current sensor output, analog signal from 0 -> vcc/2 (0A) ->vcc.                  |
| ZERO_CROSS   | 40    | PD2   | GPIO                             | Live/mains signal for sine wave zero cross detection (triac shut off point).                |
| BUZZ         | 45    | PB6   | GPIO or TIM1_CH3 or  TIM16_CH1N  | Buzzer alarm signal, frequency modulated PWM.                                               |
| DISP_CS      | 46    | PB7   | GPIO                             | SH1122 display SPI chip select signal.                                                      |
| DISP_DC      | 47    | PB8   | GPIO                             | SH1122 display data/!command signal, held low for data, high for commands.                  |
| DISP_RES     | 48    | PB9   | GPIO                             | SH1122 display reset signal.                                                                |
| TRIAC_TRIG   | 42    | PB3   | GPIO                             | Triac dimmer trigger, single pulse enables triac until instantaneous AC voltage crosses 0V. |
| HEAT_LAMP_EN | 43    | PB4   | GPIO                             | Normally open heat lamp saftey relay enable. When held high heat lamp circuit is closed.    |

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## PCB Renders

### Main Board
![image](documentation/README_IMAGES/main_board_front.png) 
![image](documentation/README_IMAGES/main_board_back.png) 

### Control Deck Board
![image](documentation/README_IMAGES/control_board_front.png) 
![image](documentation/README_IMAGES/control_board_back.png) 

### Sensor Breakout Board
See [myles-parfeniuk/wsen_hids_breakout](https://github.com/myles-parfeniuk/wsen_hids_breakout).

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Layout

### Main Board
![image](documentation/README_IMAGES/main_board_layout.png)

### Control Deck Board
![image](documentation/README_IMAGES/control_board_layout.png)

### Sensor Breakout Board
See [myles-parfeniuk/wsen_hids_breakout](https://github.com/myles-parfeniuk/wsen_hids_breakout).

<p align="right">(<a href="#readme-top">back to top</a>)</p>

## Schematics

### Main Board
<img src="hardware/biltong_board_main/biltong_board_main.svg" alt="Alt text" />

<img src="hardware/biltong_board_main/biltong_board_main-power_section.svg" alt="Alt text" />

<img src="hardware/biltong_board_main/biltong_board_main-TriacDimmer.svg" alt="Alt text" />

### Control Deck Board
<img src="hardware/biltong_board_control_deck/biltong_board_control_deck.svg" alt="Alt text" />

### Sensor Breakout Board
See [myles-parfeniuk/wsen_hids_breakout](https://github.com/myles-parfeniuk/wsen_hids_breakout).

<p align="right">(<a href="#readme-top">back to top</a>)</p>

# Firmware

The firmware is currently being developed. This repo will updated as work is completed.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

# License
Distributed under the GPL-3.0 License. See LICENSE.md for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

# Contact

Myles Parfeniuk - myles.parfenyuk@gmail.com

Project Link: [myles-parfeniuk/biltong_board](https://github.com/myles-parfeniuk/biltong_board)

<p align="right">(<a href="#readme-top">back to top</a>)</p>