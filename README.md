# About

This repository contains a collection of software codes for data collection using the LilyGO T-Call ESP32, specifically developed for applications in the smart agriculture domain. It features a robust backend built with the [succulent](https://github.com/firefly-cpp/succulent) Python framework, which serves as a web app for handling POST requests in JSON format. Additionally, the repository includes the entire ESP32 ecosystem, comprising both hardware schematics and the software that runs on ESP32 devices.

## succulent

[succulent](https://github.com/firefly-cpp/succulent) is a very lightweight framework for collecting POST requests. Based on Flask, it requires minimal hardware resources and can run practically anywhere. Users can easily modify the configuration.yml file to set up attributes that are then sent to the server or cloud. Installing [succulent](https://github.com/firefly-cpp/succulent) is straightforward and can be done using the pip Python tool or, preferably, by using a Docker image. After installation, users can modify the configuration.yml file and run the application. [succulent](https://github.com/firefly-cpp/succulent) also allows users to easily export data via a web browser and view the currently collected data in a web browser. For more information, please see the full documentation.

