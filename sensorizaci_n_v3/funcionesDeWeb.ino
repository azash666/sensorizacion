void inicializa_web(){
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent); 
  IPAddress ip2(192,168,4,1);
  dnsServer.start(53, "*", ip2);
  server.begin();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) 
{
   switch(type) {
   case WStype_DISCONNECTED:
      break;
   case WStype_CONNECTED:
      //IPAddress ip = webSocket.remoteIP(num);
      //webSocket.sendTXT(num, "Connected");
      break;
   case WStype_TEXT:
      if(payload[0]==49){
        reset_var = true;
      }
      else
        if (payload[0]<49){
          LIMITE_ACELERACION = (payload[0] * payload[0])/4;
          EEPROM.write(0, payload[0]);
          EEPROM.commit();
        }else
          if (payload[0]>49){
            LIMITE_TECHO = (payload[0]-50)*50;
            EEPROM.write(1, (payload[0]-50));
            EEPROM.commit();
          }
      //int resetea = payload[1];
      String response = ProcessRequest();
      webSocket.sendTXT(num, response);
      break;
   }
}

String ProcessRequest(){
  String parte = "{\"acelerometro\":{\"x\":";
    parte = parte + max_x;
    parte = parte + ",\"y\":";
    parte = parte + max_y;
    parte = parte + ",\"z\":";
    parte = parte + max_z;
    parte = parte + ",\"modulo_cuadrado\":";
    parte = parte + maximo_aceleracion;
    parte = parte + ",\"limite_acel_cuadrado\":";
    parte = parte + LIMITE_ACELERACION;
    parte = parte + "},\"distancia_techo\":";
    parte = parte + distancia_techo;
    parte = parte + ",\"limite_techo\":";
    parte = parte + LIMITE_TECHO;
    parte = parte + ",\"estados\":{\"interior\":";
    parte = parte + insideLed;
    parte = parte + ",\"impacto\":";
    parte = parte + golpeLed;
    parte = parte + "},\"historial\":[";
    for(int i=0; i<min(lenHistorial, posicionHistorial+1); i++){
      parte = parte + "{\"fuerza\":";
      parte = parte + historial[i];
      parte = parte + ",\"tiempo\":";
      parte = parte + ((millis()/100-tiempos[i])/10);
      parte = parte + "}";
      if (i<min(lenHistorial-1, posicionHistorial)) parte = parte + ",";
    }
    return parte+"]}";
}

void buildHTML2(WiFiClient client){
    String webpage = R"=====(
    <!DOCTYPE HTML>
    <html lang="es">
      <meta charset="utf-8" />
      <title>WEB DEL SENSOR</title>
      <style>
      .button {
        padding: 15px 32px;
        text-align: center;
        display: inline-block;
        font-size: 30px;
      }
      </style>
    </head>
    <body>
      <font size="6"><div>
        <p>Fuerza eje X (G): <span id="max_x">---</span></p>
        <p>Fuerza eje Y (G): <span id="max_y">---</span></p>
        <p>Fuerza eje Z (G): <span id="max_z">---</span></p>
        <p>Módulo de fuerza total (G): <span id="maximo_aceleracion">---</span></p>
        <p>Distancia al techo (m): <span id="distancia_techo">---</span></p>
        <p>Distancia máxima al techo (m): 
        <select id="techo" class="button" onchange="myFunction()">
        </select></p>
        <br/>
        <p>Se encuentra en interior: <span id="insideLed">---</span></p>
        <p>Se encuentra en exterior: <span id="not_insideLed">---</span></p>
        <p>Ha detectado golpe: <span id="golpeLed">---</span></p>
        <p>No ha detectado golpe: <span id="not_golpeLed">---</span></p>
        <button type="button" class="button" onclick="resetea()">Reset del golpe</button>
      </div>
      
        <p>Rango fuerza de impacto (G): 
        <select id="impacto" class="button" onchange="myFunction()">
        </select></p>
        <div id="lista"></div></font>
    <body>
    
    <script>
      var resetear = 0;
      var anterior = 0;
      var max_x = document.getElementById('max_x');
      var max_y = document.getElementById('max_y');
      var max_z = document.getElementById('max_z');
      var maximo_aceleracion = document.getElementById('maximo_aceleracion');
      var distancia_techo = document.getElementById('distancia_techo');
      var insideLed = document.getElementById('insideLed');
      var not_insideLed = document.getElementById('not_insideLed');
      var golpeLed = document.getElementById('golpeLed');
      var not_golpeLed = document.getElementById('not_golpeLed');
      var impacto = document.getElementById('impacto');
      var techo = document.getElementById('techo');
      var lista = document.getElementById('lista');
      var recibido;

      window.onload = function(){
        var inicio_techo = 0.5;
        var fin_techo = 12.0;
        var paso_techo = 0.5;
        var cadena_techo = "";
        for(var i=inicio_techo; i<=fin_techo; i+=paso_techo){
          cadena_techo = cadena_techo.concat("<option value='"+(50+i/paso_techo)+"'>"+i.toFixed(1)+"</option>");
        }
        techo.innerHTML = cadena_techo;
        
        var inicio_impacto = 0.5;
        var fin_impacto = 20.0;
        var paso_impacto = 0.5;
        var cadena_impacto = "";
        for(var i=inicio_impacto; i<=fin_impacto; i+=paso_impacto){
          cadena_impacto = cadena_impacto.concat("<option value='"+(i/paso_impacto)+"'>"+i.toFixed(1)+"</option>");
        }
        impacto.innerHTML = cadena_impacto;
      }
      
      function updateUI(counter){
        recibido= JSON.parse(counter);
        max_x.innerHTML = recibido.acelerometro.x;
        max_y.innerHTML = recibido.acelerometro.y;
        max_z.innerHTML = recibido.acelerometro.z;
        maximo_aceleracion.innerHTML = Math.sqrt(recibido.acelerometro.modulo_cuadrado).toFixed(2);
        distancia_techo.innerHTML = recibido.distancia_techo/100;
        insideLed.innerHTML = recibido.estados.interior;
        not_insideLed.innerHTML = 1-recibido.estados.interior;
        golpeLed.innerHTML = recibido.estados.impacto;
        not_golpeLed.innerHTML = 1-recibido.estados.impacto;
        if (recibido.estados.impacto != 0){
          if(anterior = 0) resetear = 0;
        }
        impacto.value = Math.round(2*Math.sqrt(recibido.acelerometro.limite_acel_cuadrado));
        techo.value = 50+2*recibido.limite_techo/100;
        //alert(impacto.value);
        anterior = recibido.estados.impacto;
        var cantidad = recibido.historial.length-1;
        var cadena = "";
        for(i=0; i<cantidad; i++){
          cadena = cadena.concat("  Fuerza = ");
          cadena = cadena.concat(Math.sqrt(recibido.historial[i].fuerza).toFixed(2));
          cadena = cadena.concat("G hace ");
          var tiempo = recibido.historial[i].tiempo;
          if(tiempo>=60){
            if(tiempo>=3600){
              cadena = cadena.concat(Math.floor(tiempo/3600));
              cadena = cadena.concat(" horas, ");
            }
            cadena = cadena.concat(Math.floor(tiempo/60)%60);
            cadena = cadena.concat(" minutos y ");
          }
          cadena = cadena.concat(tiempo%60);
          cadena = cadena.concat(" segundos<br/>");
        }
        lista.innerHTML = cadena;
      }
      
      var connection = new WebSocket('ws://' + location.hostname + ":81/", ['arduino']);
      connection.onmessage = function (e) {
        updateUI(e.data);
      }

      function resetea(){
        if(recibido.estados.impacto) connection.send(1);
      }

      impacto.onchange = function(){
        connection.send(String.fromCharCode(parseInt(impacto.value)));
      }

      
      techo.onchange = function(){
        connection.send(String.fromCharCode(parseInt(techo.value)));
      }
      
    </script>
)=====";
    
    client.println(webpage);
}
