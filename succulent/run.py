from succulent.api import SucculentAPI

api = SucculentAPI(config='configuration.yml', format='csv')

app = api.app
