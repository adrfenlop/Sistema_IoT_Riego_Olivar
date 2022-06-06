from urllib.request import urlopen
import json
from datetime import datetime
from datetime import timedelta
import boto3
from boto3.dynamodb.conditions import Key
from decimal import Decimal

def riego_normal():
    dic_ETo = {5:4.9, 6:5.73, 7:6.29, 8:5.23, 9:3.83} #Evotranspiracion de referencia en los meses de riego
    mes = int(datetime.today().strftime('%m'))
    if mes in dic_ETo:
        ETo = dic_ETo[mes]
    else:
        ETo = 5 
    Kc = 0.60 #Coeficiente de cultivo
    Sc = 40 #Superficie cubierta
    Kr = 2 * Sc / 100 #Coeficiente reductor
    ETc = ETo * Kc * Kr #Evotranspiracion del cultivo
    R = round(ETc * Sc,2) #Riego a aportar
    return R
    
def lambda_handler(event, context):
   #Inicializamos variables
    client = boto3.resource("dynamodb")
    clientMQTT = boto3.client('iot-data')
    hoy = datetime.today().strftime('%Y-%m-%d')
    riego = 0
    lluvia_ayer=0
    nodo=int(event["ID"])
    humedad=int(event["Humedad"])
    
    #Acceso a DynamoDB tabla predicciones meteorologicas        
    table = client.Table("prediccion_meteo")
    
    #Obtenemos la predicción de ayer almacenada en la tabla
    ayer = datetime.today() - timedelta(days = 1)
    response = table.query(KeyConditionExpression=Key('Fecha').eq(ayer.strftime('%Y-%m-%d')))#Buscamos el indice con la fecha de ayer
    if len(response['Items']) > 0:
        dato = response['Items'][0]
        if dato["Fecha"] == ayer.strftime('%Y-%m-%d'):    
            lluvia_ayer= dato["Lluvia"]
    else:
        lluvia_ayer = 0
        
    #Tomamos la decision en funcion de la prediccion de la lluvia de ayer y la humedad del nodo
    if lluvia_ayer > 0 & humedad==1:
        riego = 0
    else:
        riego = riego_normal()#Litros/Olivo
            
    #Publicamos mensaje MQTT
    s = 'riego/'+str(nodo)
    clientMQTT.publish(topic= s,qos=1,payload=str(riego))
    
    #Acceso a DynamoDB tabla predicciones meteorologicas        
    table = client.Table("decisionesRiego")
    
    #Almacenamos la decision en DynamoDB
    data = {'Nodo': nodo,'Fecha': hoy,'Riego': riego}
    ddb_data = json.loads(json.dumps(data), parse_float=Decimal)
    response = table.put_item(Item=ddb_data)
            
    return None
