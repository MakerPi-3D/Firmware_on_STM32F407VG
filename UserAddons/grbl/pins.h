#ifndef PINS_H
#define PINS_H

#define digitalRead(ulPin) HAL_GPIO_ReadPin(ulPin##_GPIO, ulPin)
#define digitalWrite(ulPin, ulVal) HAL_GPIO_WritePin(ulPin##_GPIO, ulPin, (GPIO_PinState)ulVal)

#define STEP_START_PIN_GPIO         GPIOB
#define STEP_START_PIN              GPIO_PIN_3
#define STEP_START_PIN_ON           1

#define X_STEP_PIN_GPIO             GPIOC
#define X_STEP_PIN                  GPIO_PIN_5
#define X_STEP_PIN_ON               1

#define X_DIR_PIN_GPIO              GPIOA
#define X_DIR_PIN                   GPIO_PIN_3
#define X_DIR_PIN_ON                1

#define X_ENABLE_PIN_GPIO           GPIOC
#define X_ENABLE_PIN                GPIO_PIN_3
#define X_ENABLE_PIN_ON             1

#define X_MIN_PIN_GPIO              GPIOC
#define X_MIN_PIN                   GPIO_PIN_2
#define X_MIN_PIN_ON                1

#define X_MAX_PIN_GPIO              GPIOF
#define X_MAX_PIN                   GPIO_PIN_4
#define X_MAX_PIN_ON                -1


#define Y_STEP_PIN_GPIO             GPIOD
#define Y_STEP_PIN                  GPIO_PIN_11
#define Y_STEP_PIN_ON               1

#define Y_DIR_PIN_GPIO              GPIOB
#define Y_DIR_PIN                   GPIO_PIN_12
#define Y_DIR_PIN_ON                1

#define Y_ENABLE_PIN_GPIO           GPIOB
#define Y_ENABLE_PIN                GPIO_PIN_13
#define Y_ENABLE_PIN_ON             1

#define Y_MIN_PIN_GPIO              GPIOA
#define Y_MIN_PIN                   GPIO_PIN_4
#define Y_MIN_PIN_ON                1

#define Y_MAX_PIN_GPIO              GPIOF
#define Y_MAX_PIN                   GPIO_PIN_9
#define Y_MAX_PIN_ON                -1

#define Z_STEP_PIN_GPIO             GPIOC
#define Z_STEP_PIN                  GPIO_PIN_7
#define Z_STEP_PIN_ON               1

#define Z_DIR_PIN_GPIO              GPIOB
#define Z_DIR_PIN                   GPIO_PIN_7
#define Z_DIR_PIN_ON                1

#define Z_ENABLE_PIN_GPIO           GPIOD
#define Z_ENABLE_PIN                GPIO_PIN_6
#define Z_ENABLE_PIN_ON             1

#define Z_MIN_PIN_GPIO              GPIOA
#define Z_MIN_PIN                   GPIO_PIN_0
#define Z_MIN_PIN_ON                1

#define Z_MAX_PIN_GPIO              GPIOA
#define Z_MAX_PIN                   GPIO_PIN_15
#define Z_MAX_PIN_ON                1


#define E0_STEP_PIN_GPIO            GPIOE
#define E0_STEP_PIN                 GPIO_PIN_3
#define E0_STEP_PIN_ON              1

#define E0_DIR_PIN_GPIO             GPIOE
#define E0_DIR_PIN                  GPIO_PIN_5
#define E0_DIR_PIN_ON               1

#define E0_ENABLE_PIN_GPIO          GPIOE
#define E0_ENABLE_PIN               GPIO_PIN_1
#define E0_ENABLE_PIN_ON            1


#define E1_STEP_PIN_GPIO            GPIOE
#define E1_STEP_PIN                 GPIO_PIN_2
#define E1_STEP_PIN_ON              1

#define E1_DIR_PIN_GPIO             GPIOE
#define E1_DIR_PIN                  GPIO_PIN_0
#define E1_DIR_PIN_ON               1

#define E1_ENABLE_PIN_GPIO          GPIOE
#define E1_ENABLE_PIN               GPIO_PIN_4
#define E1_ENABLE_PIN_ON            1

#define MOTORXY_MS3_PIN_GPIO        GPIOB
#define MOTORXY_MS3_PIN             GPIO_PIN_11
#define MOTORXY_MS3_PIN_ON          1

#define MOTORZ_MS3_PIN_GPIO         GPIOB
#define MOTORZ_MS3_PIN              GPIO_PIN_10
#define MOTORZ_MS3_PIN_ON           1

#define MOTORE_MS3_PIN_GPIO         GPIOA
#define MOTORE_MS3_PIN              GPIO_PIN_8
#define MOTORE_MS3_PIN_ON           1

#define HEATER_BED_PIN_GPIO         GPIOD
#define HEATER_BED_PIN              GPIO_PIN_12
#define HEATER_BED_PIN_ON           1

#define HEATER_0_PIN_GPIO           GPIOD
#define HEATER_0_PIN                GPIO_PIN_13
#define HEATER_0_PIN_ON             1

#define HEATER_1_PIN_GPIO           GPIOB
#define HEATER_1_PIN                GPIO_PIN_9
#define HEATER_1_PIN_ON             -1

#define HEATER_CAVITY_PIN_GPIO             GPIOB
#define HEATER_CAVITY_PIN                  GPIO_PIN_4
#define HEATER_CAVITY_PIN_ON               1

// #define HEATER_2_PIN_GPIO           GPIOD
//  #define HEATER_2_PIN              GPIO_PIN_3
#define HEATER_2_PIN_ON       -1

#define TEMP_BED_PIN_ADC_CHANNEL    ADC_CHANNEL_7
#define TEMP_BED_PIN_ADC            ADC1
#define TEMP_BED_PIN_GPIO           GPIOA
#define TEMP_BED_PIN_GPIO_CLK       RCC_AHB1Periph_GPIOA
#define TEMP_BED_PIN                GPIO_PIN_7
#define TEMP_BED_PIN_ON             1

#define TEMP_0_PIN_ADC_CHANNEL      ADC_CHANNEL_6
#define TEMP_0_PIN_ADC              ADC1
#define TEMP_0_PIN_GPIO             GPIOA
#define TEMP_0_PIN_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define TEMP_0_PIN                  GPIO_PIN_6
#define TEMP_0_PIN_ON               1

// #define TEMP_1_PIN_ADC_CHANNEL      ADC_Channel_2
// #define TEMP_1_PIN_ADC              ADC1
// #define TEMP_1_PIN_GPIO            GPIOA
//  #define TEMP_1_PIN_GPIO_CLK        RCC_AHB1Periph_GPIOA
//  #define TEMP_1_PIN               GPIO_PIN_2
#define TEMP_1_PIN_ON        -1

// #define TEMP_2_PIN_ADC_CHANNEL      ADC_Channel_3
// #define TEMP_2_PIN_ADC              ADC1
// #define TEMP_2_PIN_GPIO            GPIOA
//  #define TEMP_2_PIN_GPIO_CLK        RCC_AHB1Periph_GPIOA
// #define TEMP_2_PIN               GPIO_PIN_3
#define TEMP_2_PIN_ON        -1

// 腔体与断料共用IO PA5
#define TEMP_CAVITY_PIN_ADC_CHANNEL      ADC_CHANNEL_5
#define TEMP_CAVITY_PIN_ADC              ADC1
#define TEMP_CAVITY_PIN_GPIO             GPIOA
#define TEMP_CAVITY_PIN_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define TEMP_CAVITY_PIN                  GPIO_PIN_5
#define TEMP_CAVITY_PIN_ON               1

#define MATCheck_PIN_ADC_CHANNEL      ADC_CHANNEL_5
#define MATCheck_PIN_ADC              ADC1
#define MATCheck_PIN_GPIO             GPIOA
#define MATCheck_PIN_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define MATCheck_PIN                  GPIO_PIN_5
#define MATCheck_PIN_ON               1

#define MOTOR_FAN_PIN_GPIO            GPIOC
#define MOTOR_FAN_PIN                 GPIO_PIN_15
#define MOTOR_FAN_PIN_ON              1

#define BOARD_FAN_PIN_GPIO            GPIOC
#define BOARD_FAN_PIN                 GPIO_PIN_6
#define BOARD_FAN_PIN_ON              1

#define FAN_PIN_SOURCE                GPIO_PinSource1
#define FAN_PIN_TIMER_CHANNEL         TIM_Channel_2
#define FAN_PIN_PWM_TIMER             TIM5
#define FAN_PIN_GPIO                  GPIOA
#define FAN_PIN_GPIO_CLK              RCC_AHB1Periph_GPIOA
#define FAN_PIN                       GPIO_PIN_1
#define FAN_PIN_ON                    1

#define BEEPER_PIN_GPIO               GPIOA
#define BEEPER_PIN                    GPIO_PIN_2
#define BEEPER_PIN_ON                 1

#endif //PINS_H

