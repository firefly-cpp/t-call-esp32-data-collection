# About

This repository contains a collection of software codes for data collection using the LilyGO T-Call ESP32, specifically developed for applications in the smart agriculture domain. It features a robust backend built with the [succulent](https://github.com/firefly-cpp/succulent) Python framework, which serves as a web app for handling POST requests in JSON format. Additionally, the repository includes the entire ESP32 ecosystem, comprising both hardware schematics and the software that runs on ESP32 devices.

## succulent

[succulent](https://github.com/firefly-cpp/succulent) is a very lightweight framework for collecting POST requests. Based on Flask, it requires minimal hardware resources and can run practically anywhere. Users can easily modify the configuration.yml file to set up attributes that are then sent to the server or cloud. Installing [succulent](https://github.com/firefly-cpp/succulent) is straightforward and can be done using the pip Python tool or, preferably, by using a Docker image. After installation, users can modify the configuration.yml file and run the application. [succulent](https://github.com/firefly-cpp/succulent) also allows users to easily export data via a web browser and view the currently collected data in a web browser. For more information, please see the full documentation.

### 🐳 succulent Container (How to Use?)

Create a `docker-compose.yml` file with the following content in the root directory:

```yml
version: '3.8'

services:
  app:
    image: codeberg.org/firefly-cpp/succulent:v3
    ports:
      - "8080:8080"
    volumes:
      - ./run.py:/succulent-app/run.py
      - ./configuration.yml:/succulent-app/configuration.yml
    environment:
      - GUNICORN_WORKERS=2
```

Next create a `configuration.yml` file in the root directory. Here's an example of a configuration file:

```yml
data:
  - name: 'temperature'
  - name: 'humidity'
  - name: 'moisture'
  - name: 'light'
results:
  - enable: true # false by default | to see the results in web browser
```

More information regarding the configuration file and its settings can be found in the [succulent](https://github.com/firefly-cpp/succulent) repository.

Then create a Python file named `run.py` with the following content in the root directory:

```python
from succulent.api import SucculentAPI

api = SucculentAPI(config='configuration.yml', format='csv')

app = api.app
```

Once you have set up the configuration file and the Python file, build the Docker image with the following command:

```bash
docker compose build
```

Finally, run the Docker container with the following command:

```bash
docker compose up
```

These steps also apply if you're using Podman instead of Docker.
