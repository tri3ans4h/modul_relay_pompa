# waterlevelserver


Modul B
Modul ini di gunakan pada sisi server

Rencana Hardware:
- arduino nano v3
- esp8266 v01
- LM2596 DC to DC step down
- relay
- Adaptor 5 volt

Rencana Software :
- Esp8266 Server connect to router 
- Esp8266 kirim data ke Modul B setiap 5 detik, format command=sensor
- Esp8266 baca data yang dikirim Modul A dalam format sensor=10
- Berdasar jarak yang dibaca kemudian dihitung volume yang mungkin
- volume = luas_alas x tinggi
- luas_alas = phi x r x r
- tinggi(cm)= tinggi_tandon_air(cm) - nilai_sensor(cm)
- r = diameter tandon
- misal : 
    - r = 50 cm
    - tinggi_tandon_air = 150 cm
    - nilai_sensor = 100 cm
    - tinggi = 150 - 100 = 50 cm
    - luas_alas = 22/7 x 50 x 50 = 7857.14 cm2
    - volume = 7857.14 x 50 = 392857.143 cm3 = 392.857143 liter


Modifikasi Modul
    - Untuk kendali lampu dengan memanfaatkan relay
    - hilangkan fungsi calcVolume
    
Contoh :

Request : command=relay
Response : command=relay&value=1
    
Request : command=relayon
Response : command=relayon&value=1

Request : command=relayoff
Response : command=relayoff&value=0

secara interval mengirim perintah
Request : command=sensor menuju modul sensor
Response : command=sensor&value=10
      
Credit :
- http://arduino-er.blogspot.co.id/2015/05/arduino-esp8266-web-server-ii.html
- http://www.martyncurrey.com/arduino-esp8266-web-server/
