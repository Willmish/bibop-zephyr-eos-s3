# Unbricking/reflashing bootloader for Quicklogic QT Plus/Quickfeather

https://forum.quicklogic.com/viewtopic.php?t=29

# Building the application for QuickLogic QT Plus:

We decided to update in-tree drivers and Modules, which in hindsight will be nice for upstreaming, but a pain for develpoing (need to keep 3 repos)

You will need the updated HAL module from [here](https://github.com/Willmish/hal_quicklogic/tree/hal_i2c)
And the updated zephyr repo with the I2C driver: [here](https://github.com/Willmish/zephyr/tree/eos_s3_fixes).

We are using an external module (hal_quicklogic), which is linked and built with the flag `-DEXTRA_ZEPHYR_MODULES`. However although docs mention this module will take precedence above the module intree. However, when building it tries building both and gives errors of redefinition, so you will need to remove the quicklogic hal old module from:
```
rm -rf zephyrproject/modules/hal/quicklogic
```

then build it with: 
```
west build -p always -b quick_feather app/ -- -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DEXTRA_ZEPHYR_MODULES=/path/to/cloned/hal_quicklogic/
```

FLash with patched tinyFPGAProgrammer: https://github.com/QuickLogic-Corp/TinyFPGA-Programmer-Application/

# Zephyr Example Application

This repository contains a Zephyr example application. The main purpose of this
repository is to serve as a reference on how to structure Zephyr-based
applications. Some of the features demonstrated in this example are:

- Basic [Zephyr application][app_dev] skeleton
- [Zephyr workspace applications][workspace_app]
- [Zephyr modules][modules]
- [West T2 topology][west_t2]
- [Custom boards][board_porting]
- Custom [devicetree bindings][bindings]
- Out-of-tree [drivers][drivers]
- Out-of-tree libraries
- Example CI configuration (using Github Actions)
- Custom [west extension][west_ext]

This repository is versioned together with the [Zephyr main tree][zephyr]. This
means that every time that Zephyr is tagged, this repository is tagged as well
with the same version number, and the [manifest](west.yml) entry for `zephyr`
will point to the corresponding Zephyr tag. For example, the `example-application`
v2.6.0 will point to Zephyr v2.6.0. Note that the `main` branch always
points to the development branch of Zephyr, also `main`.

[app_dev]: https://docs.zephyrproject.org/latest/develop/application/index.html
[workspace_app]: https://docs.zephyrproject.org/latest/develop/application/index.html#zephyr-workspace-app
[modules]: https://docs.zephyrproject.org/latest/develop/modules.html
[west_t2]: https://docs.zephyrproject.org/latest/develop/west/workspaces.html#west-t2
[board_porting]: https://docs.zephyrproject.org/latest/guides/porting/board_porting.html
[bindings]: https://docs.zephyrproject.org/latest/guides/dts/bindings.html
[drivers]: https://docs.zephyrproject.org/latest/reference/drivers/index.html
[zephyr]: https://github.com/zephyrproject-rtos/zephyr
[west_ext]: https://docs.zephyrproject.org/latest/develop/west/extensions.html

## Getting Started

Before getting started, make sure you have a proper Zephyr development
environment. Follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

### Initialization

The first step is to initialize the workspace folder (``my-workspace``) where
the ``example-application`` and all Zephyr modules will be cloned. Run the following
command:

```shell
# initialize my-workspace for the example-application (main branch)
west init -m https://github.com/zephyrproject-rtos/example-application --mr main my-workspace
# update Zephyr modules
cd my-workspace
west update
```

### Building and running


To build the application, run the following command:

```shell
west build -b $BOARD app
```

where `$BOARD` is the target board.

You can use the `custom_plank` board found in this
repository. Note that Zephyr sample boards may be used if an
appropriate overlay is provided (see `app/boards`).

A sample debug configuration is also provided. To apply it, run the following
command:

```shell
west build -b $BOARD app -- -DOVERLAY_CONFIG=debug.conf
```

Once you have built the application, run the following command to flash it:

```shell
west flash
```
