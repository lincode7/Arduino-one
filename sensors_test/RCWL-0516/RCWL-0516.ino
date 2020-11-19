int pre = 0;
int cur = 0;
void setup()
{
  Serial.begin(9600);
    pinMode(2,INPUT);
    pinMode(13,OUTPUT);
}

void loop() {
  if (cur = digitalRead(2)) {
    digitalWrite(13,LOW);
    Serial.printf("pre:%d, cur:%d\n",pre,cur);
    pre = cur;
  }
  else {
    digitalWrite(13,HIGH);
    Serial.println("No montion");
    pre = cur;
    delay(1500);
  }
}
