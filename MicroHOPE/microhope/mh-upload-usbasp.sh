echo "Uploading  $1.hex to ATmega32 on microHOPE via ISP using USBASP programmer"
avrdude -B10 -c usbasp -patmega32 -U flash:w:$1.hex    # upload hex file
