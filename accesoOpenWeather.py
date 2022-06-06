from urllib.request import urlopen
import json
from datetime import datetime
from datetime import timedelta
import boto3
from boto3.dynamodb.conditions import Key
from decimal import Decimal

def lambda_handler(event, context):
    hoy = datetime.today().strftime('%Y-%m-%d')
    client = boto3.resource("dynamodb")
    #Acceso a DynamoDB tabla predicciones meteorologicas        
    table = client.Table("prediccion_meteo")
    
    #Almacenamos la prediccion para hoy preguntando a la API de OpenWeather
    url = 'https://api.openweathermap.org/data/2.5/onecall?lat=37.28&lon=-5.49&exclude=current,minutely,hourly&appid='
    response = urlopen(url)
    string = response.read().decode('utf-8')
    data = json.loads(string)
    
    for dia in data["daily"]:
        fecha = datetime.fromtimestamp(dia["dt"]).strftime('%Y-%m-%d')
        if(fecha == hoy):
            if "rain" in dia:
                data = {'Fecha': fecha,'Lluvia': dia["rain"]}
            else:
                data = {'Fecha': fecha,'Lluvia': 0}
            print(data)
            ddb_data = json.loads(json.dumps(data), parse_float=Decimal)
            response = table.put_item(Item=ddb_data)
            
    return 0