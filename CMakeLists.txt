#此文件从模板自动生成! 请勿更改!
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.23)

# specify cross-compilers and tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# project settings
project(f407zg_gcc_demo C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

#Uncomment for hardware floating point
add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

#Uncomment for software floating point
#add_compile_options(-mfloat-abi=soft)

add_compile_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)

# uncomment to mitigate c++17 absolute addresses warnings
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

# Enable assembler files preprocessing
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
endif ()

include_directories(
        app
        board_dev
        gcc_port
        mcu_dev
        StLib/CMSIS/Core/Include
        StLib/STM32F4xx_StdPeriph_Driver/inc
        middlewares/littlefs
        middlewares/sfud
        middlewares/printf_

        #/*FreeRTOS*/
        middlewares/FreeRTOS-Kernel/include
        middlewares/FreeRTOS-Kernel/portable/GCC/ARM_CM4F


        #fatfs
        middlewares/fatfs_R0.15
   #     Core/Inc Drivers/STM32F4xx_HAL_Driver/Inc Drivers/STM32F4xx_HAL_Driver/Inc/Legacy Drivers/CMSIS/Device/ST/STM32F4xx/Include Drivers/CMSIS/Include)
)
#add_definitions(-DUSE_HAL_DRIVER -DSTM32F407xx)

file(GLOB_RECURSE SOURCES
       app/*.*
        board_dev/*.*
        gcc_port/startup_stm32f407xx.s
        gcc_port/syscalls.c
        mcu_dev/*.*
        StLib/STM32F4xx_StdPeriph_Driver/src/*.*
        middlewares/littlefs/*.*
        middlewares/sfud/*.*
        middlewares/printf_/printf.c

        #/*FreeRTOS*/
        middlewares/FreeRTOS-Kernel/croutine.c
        middlewares/FreeRTOS-Kernel/event_groups.c
        middlewares/FreeRTOS-Kernel/list.c
        middlewares/FreeRTOS-Kernel/queue.c
        middlewares/FreeRTOS-Kernel/stream_buffer.c
        middlewares/FreeRTOS-Kernel/tasks.c
        middlewares/FreeRTOS-Kernel/timers.c
        middlewares/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c
        middlewares/FreeRTOS-Kernel/portable/MemMang/heap_4.c

        #fatfs
        middlewares/fatfs_R0.15/*.c

)
file(GLOB_RECURSE EXCLUDE_SRCS
        StLib/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmc.c
        )
# 从源文件列表(SOURCES)中剔除干扰文件(EXCLUDE_SRCS)
list(REMOVE_ITEM SOURCES ${EXCLUDE_SRCS})

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/gcc_port/STM32F407ZGTx_FLASH.ld)

add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_link_options(-T ${LINKER_SCRIPT})

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")
