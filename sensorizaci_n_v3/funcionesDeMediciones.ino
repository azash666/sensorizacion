void inicializa_mediciones(){
    tfmini.begin(&mySerial);
    inicializa_acelerometro();
    siguienteComprobacionTiempo = millis() + TIEMPO_ENTRE_LECTURAS;
}

int medirDistanciaLiDAR() {
    int dist = (int) tfmini.getDistance();
    return dist>0?dist:50000;
}

long medirDistancia(int trigPin, int echoPin) {
    long distancia = 5;
    long duration;
    digitalWrite(trigPin, LOW);
    delayMicroseconds(4);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distancia = duration * 10 / 292 / 2;
    return distancia;
}

void inicializa_acelerometro() {
    
    Wire.begin();
    Wire.beginTransmission(0x0A); // address of the accelerometer
    // range settings
    Wire.write(0x22); //register address
    Wire.write(0x03); //can be set at"0x00""0x01""0x02""0x03", refer to Datashhet on wiki
    // low pass filter
    Wire.write(0x20); //register address
    Wire.write(0x05); //can be set at"0x05""0x04"......"0x01""0x00", refer to Datasheet on wiki
    Wire.endTransmission();
}

int lee_acelerometro() {
    byte Version[3];
    int8_t x_data;
    int8_t y_data;
    int8_t z_data;

    int divi=2;
   // int x,y,z;

    Wire.beginTransmission(0x0A); // address of the accelerometer
    // reset the accelerometer
    Wire.write(0x04); // Y data
    Wire.endTransmission();
    Wire.requestFrom(0x0A,1);    // request 6 bytes from slave device #2
    while(Wire.available())    // slave may send less than requested
    {
      Version[0] = Wire.read(); // receive a byte as characte
    }
    x_data=(int8_t)Version[0]>>2;
  
    Wire.beginTransmission(0x0A); // address of the accelerometer
    // reset the accelerometer
    Wire.write(0x06); // Y data
    Wire.endTransmission();
    Wire.requestFrom(0x0A,1);    // request 6 bytes from slave device #2
    while(Wire.available())    // slave may send less than requested
    {
      Version[1] = Wire.read(); // receive a byte as characte
    }
    y_data=(int8_t)Version[1]>>2;
  
    Wire.beginTransmission(0x0A); // address of the accelerometer
    // reset the accelerometer
    Wire.write(0x08); // Y data
    Wire.endTransmission();
    Wire.requestFrom(0x0A,1);    // request 6 bytes from slave device #2
     while(Wire.available())    // slave may send less than requested
    {
      Version[2] = Wire.read(); // receive a byte as characte
    }
    z_data=(int8_t)Version[2]>>2;
    x=(float)x_data/divi+.5;
    y=(float)y_data/divi +1;
    z=(float)z_data/divi;
    
    int modulo_cuadrado = x * x + y * y + z * z;
    return modulo_cuadrado;
}
