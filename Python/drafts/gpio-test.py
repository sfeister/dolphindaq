import Adafruit_BBIO.GPIO as GPIO

PIN_LED = "P8_7"

GPIO.setup(PIN_LED, GPIO.OUT)
GPIO.output(PIN_LED, GPIO.HIGH)
print(GPIO.input(PIN_LED))
GPIO.cleanup()