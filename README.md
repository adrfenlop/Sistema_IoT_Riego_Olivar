# Sistema_IoT_Riego_Olivar
Sistema IoT para el riego óptimo del olivar basado en servicios abiertos de predicción meteorológica
Los documentos accionNodo y accesoOpenWeather definen funciones lambda a implementar en AWS, en el manual de despliegue se encuentran las explicaciones de como implementarlas.

Se proporcionan 3 códigos diferentes para los nodos:
  - connectAWS. El código básico con toda la funcionalidad necesaria para el funcionamiento de los nodos dentro del sistema.
  - connectAWS_OLED. Incluye la funcionalidad básica más la muestra por pantalla de información relativa a la ejecución del programa. Usado en la versión con pantalla OLED.
  - connectAWS_Debug. Define la funcionalidad básica más mensaje de debug por el puerto serie a lo largo de la ejecución del programa.

librerias_arduino contiene todas las librerias utilizadas para los 3 códigos utilizados en los nodos.

datos_analisis contiene los archivos extraidos de la base de datos para ejemplificar los analisis del manual de usuario.

Caja y Tapadera son los archivos stl utilizados para la impresión 3D del encapsulado de los nodos.
