const int LED = 1; //pin untuk LED
const int rowPins[numRows] = {A5, A4, A3, A2}; //pin keypad
const int colPins[numCols] = {A1, A0, 6}; //pin keypad
LiquidCrystal lcd (7, 8, 5, 4, 3, 2); //pin LCD

void setup()
{
  radio.begin(); //memulai radio
  radio.setRetries(15,15); //delay antar retry
  lcd.begin(16,2); //memulai lcd dengan ukuran 16x2
  pinMode(LED, OUTPUT); //LED sebagai output
  radio.openWritingPipe(pipes[0]); //membuka pipa radio untuk menulis
  radio.openReadingPipe(1,pipes[1]); //membuka pipa radio pada posisi 1 untuk membaca 
  radio.startListening(); //radio mulai mendengarkan
  randomSeed(analogRead(0)); //fungsi untuk random
  for (int row = 0; row < numRows; row++)
  {
    pinMode(rowPins[row],INPUT); //set pin baris sebagai input
    digitalWrite(rowPins[row],HIGH); //menyalakan pull-up
  }
  for (int column = 0; column < numCols; column++)     
  {     
     pinMode(colPins[column],OUTPUT); //set pin kolom sebagai output     
     digitalWrite(colPins[column],HIGH); //semua kolom tidak aktif  
  } 
  lcd.setCursor(0,0); //set posisi kursor LCD 
}

void loop() {
   if (isReadyP1 == true){ //jika P1 sudah siap bermain
     digitalWrite(LED, HIGH); //LED menyala   
   }   
   else if (isReadyP1 == false){ //jika P1 belum siap 
     lcd.setCursor(0,0); //set posisi kursor
     lcd.print("Press Enter to"); //tampilkan tulisan
     lcd.setCursor(0,1); //set posisi kursor ke baris 2
     lcd.print("begin"); //tampilkan tulisan
     digitalWrite(LED, LOW); //LED tidak menyala   
   }
   if (isReady == false){ //jika ada player yang belum siap
     radio.read(&rcv, sizeof(int)); //membaca rcv yang diterima
     areYouReady(); //menjalankan prosedur untuk mengecek apakah pemain telah siap
   }
   else if (isReady == true) //kedua pemain telah siap
   {
     isOver = false; //isi isOver dengan nilai false
     digitalWrite(LED, LOW); //matikan LED
     lcd.clear(); //bersihkan layar LCD
     generateSoal(); //generate soal
     t.after(60000, gameOver); //setelah 60detik permainan akan dijalankan prosedur gameOver
     lcd.setCursor(6,0); //set posisi kursor ke kolom 6
     do{
       radio.read(&rcv, sizeof(int)); //membaca rcv yang diterima
       t.update(); //timer terus di update
       char key = getKey(); //membaca tombol yang ditekan
       if(rcv == 98){ //jika kode rcv adalah 98
         isSubmitted = true; //berarti jawaban telah di submit
       }
       if(key != 0) //ulangi selama ada tombol yang ditekan 
      { 
         if (key != '#' && key != '*'){ //jika yang ditekan bukan tombol # atau *
            sans += key; //simpan ke dalam string
           lcd.print(key); //cetak pada lcd
           key = getKey(); //baca lagi apakah ada tombol yang ditekan
         }
         else if (key == '*'){ //jika ditekan tombol *
           sans = ""; //kosongkan isi variabel sans
           lcd.setCursor(6,0); //set posisi kursor ke kolom 6 
           lcd.print("        "); //replace angka sebelumnya dengan spasi agar terhapus pada layar
           lcd.setCursor(6,0); //set posisi kursor ke kolom 6 lagi
         }         else if (key == '#'){ //tombol # ditekan
           trans = 98;  //isi kode trans dengan 98 sebagai kode
           radio.stopListening();   //berhenti mendengarkan agar dapat menulis
           radio.write(&trans, sizeof(int)); //menulis/mengirimkan trans
           radio.startListening(); //kembali mendengarkan
           radio.read(&rcv,sizeof(int)); //membaca rcv yang diterima
           isSubmitted = true; //isi isSubmitted dengan nilai true
           ans = sans.toInt(); //konversi jawaban dari String ke integer
         }
      }
      if (isSubmitted==true) //pemain men-submit jawaban
      {
         if(rcv == 98){ //kode rcv yang diterima adalah 98
           lcd.setCursor(0,1); //set posisi kursor ke kolom 2
           lcd.print("P2 has answered"); //tampilkan tulisan
           rcv = 0; //isi rcv dengan nilai 0
         }
         else{
           cek(ans); //periksa jawaban
           if(isRight==true){ //jawaban benar
             skor = skor + 10; //skor bertambah
             lcd.print("That's Right!"); //cetak tulisan
             digitalWrite(LED, HIGH); //LED menyala sebagai indikator jawaban benar
           }
           else{ //jawaban salah
             if(skor >= 5){ //jika skor >= 5
              skor = skor - 5; //skor dikurangi
            } //jika skor kurang dari 5 maka tidak akan ada pengurangan skor
            lcd.print("That's Wrong"); //cetak tulisan
          }
        }
        delay(1500); //jeda 1.5 detik
        digitalWrite(LED, LOW);
        sans = ""; //kosongkan string jawaban
        ans = 0; //set jawaban = 0
        lcd.clear(); //bersihkan layar LCD
        generateSoal(); //generate soal lagi
        lcd.setCursor(6,0); //set posisi kursor ke kolom 6
        isSubmitted=false; //ganti iSubmitted ke nilai false
      }
    } while(!isOver); //lakukan selama game belum berakhir
  }
}

void areYouReady(){ //prosedur untuk mengecek apakah pemain sudah siap
  char key = getKey(); //menerima input dari keypad
  if(key != 0) //jika ada tombol yang ditekan
  { 
    if (key == '#'){ //tombol # ditekan
      isReadyP1 = true; //artinya P1 sudah siap
      lcd.clear(); //bersihkan layar LCD
      lcd.setCursor(0,0); //set posisi kursor
      lcd.print("Waiting for P2"); //tampilkan tulisan
      trans = 99; //kode trans yang akan digunakan
      radio.stopListening(); //berhenti mendengarkan agar dapat menulis
      radio.write(&trans,sizeof(int)); //mengirimkan trans
      radio.startListening(); //kembali mendengarkan
      radio.read(&rcv,sizeof(int)); //membaca kode rcv yang diterima
    }
  }
  if(rcv == 99){ //jika menangkap angka 99
    isReadyP2 = true; //artinya P2 sudah siap
  }
  if(isReadyP1 && isReadyP2){ //kedua pemain sudah siap
    digitalWrite(LED, HIGH); //LED tetap menyala
    isReady = true; //isi isReady dengan nilai true
    lcd.clear(); //bersihkan layar LCD
    lcd.setCursor(0,0); //set posisi kursor
    lcd.print("You'll get +10"); //cetak petunjuk
    lcd.setCursor(0,1); //set posisi kursor ke baris 2
    lcd.print("for right answer"); //cetak petunjuk
    delay(2000); //jeda untuk membaca petunjuk
    lcd.clear(); //bersihkan layar LCD
    lcd.setCursor(0,0); //set posisi kursor
    lcd.print("And -5"); //cetak petunjuk
    lcd.setCursor(0,1);//set posisi kursor ke baris 2
    lcd.print("for wrong answer"); //cetak petunjuk
    delay(2000); //jeda untuk membaca petunjuk
    lcd.clear(); //bersihkan layar LCD
    lcd.setCursor(0,0); //set posisi kursor
    lcd.print("You have"); //cetak petunjuk
    lcd.setCursor(0,1); //set posisi kursor ke baris 2
    lcd.print("60 seconds"); //cetak petunjuk
    delay(1500); //jeda untuk membaca petunjuk
    lcd.clear(); //bersihkan layar LCD
    lcd.print("Game starts"); //cetak petunjuk
    delay(1000); //jeda untuk membaca petunjuk
    lcd.print(" NOW!!!"); //cetak petunjuk
    delay(1000); //jeda untuk membaca petunjuk
    digitalWrite(LED, LOW); //matikan LCD
  }
}

void gameOver(){  //melakukan beberapa aksi setelah permainan berakhir
  isOver = true; //isi isOver dengan nilai true
  radio.stopListening(); //berhenti mendengarkan agar dapat menulis
  radio.write(&skor,sizeof(int)); //mengirimkan skor
  radio.startListening(); //kembali mendengarkan
  nilai = 0; //isi nilai dengan 0 untuk membantu filtering
  do{ //lakukan filtering agar angka yang disimpan hanya skor lawan
    radio.read(&skormusuh,sizeof(int));
    if((nilai < skormusuh) && (nilai != 98) && (nilai != 99)){
      a = skormusuh % 5; //dilakukan filter skormusuh yang masuk
      if(a == 0){ //skormusuh kelipatan 5
        nilai = skormusuh; //isi nilai dengan skor lawan
      }
    }
  } while(nilai <= skormusuh); //lakukan terus selama nilai <= skormusuh
   radio.stopListening(); //berhenti mendengarkan agar dapat menulis
   radio.write(&skor, sizeof(int)); //mengirimkan skor
   radio.startListening(); //kembali mendengarkan
   lcd.clear(); //bersihkan layar LCD
   lcd.setCursor(0,0); //set posisi kursor
    lcd.print("Time's up!!!"); //tampilkan tulisan
   lcd.setCursor(0,1); //set posisi kursor ke baris 2
   if(nilai == skor){ //skor P1 = skor P2
     lcd.print("It's a draw!");//tampilkan tulisan
   }
   else if(nilai > skor){ //pemain kalah
    lcd.print("You lose!");  //tampilkan tulisan
  }
  else if(nilai < skor){ //pemain menang
    lcd.print("You Win!");//tampilkan tulisan
  }
  delay(3000); //jeda waktu untuk membaca tulisan
  lcd.clear(); //bersihkan layar LCD
  lcd.setCursor(0,0); //set posisi kursor
  lcd.print("Your Score: "); //cetak tulisan
  lcd.print(skor); //cetak skor
  lcd.setCursor(0,1); //set posisi kursor ke baris 2
  lcd.print("Rival Score: "); //cetak tulisan
  lcd.print(nilai); //cetak skor musuh
  delay(3000); //jeda untuk membaca tulisan
  isReadyP1 = false; //isi isReadyP1 dengan nilai false
  isReadyP2 = false; //isi isReadyP2 dengan nilai false
  isReady = false; //isi isReady dengan nilai false
  lcd.clear(); //bersihkan layar LCD
}

void generateSoal(){ //prosedur untuk generate soal
  lcd.setCursor(0, 0); // set posisi kursor
  firstNumber = random(10,50); //angka pertama
  lcd.print(firstNumber); //cetak angka pertama
  i = random(1); //random nilai indeks i
  lcd.print(operan[i]); //random operator
  secondNumber = random(10,50); //angka kedua
  lcd.print(secondNumber); //cetak angka kedua
  lcd.print("="); //cetak "="
}

void cek(int ans){ //prosedur untuk memeriksa jawaban pemain
  switch (operan[i]){
  case '+' : //jika operan adalah +
    kunci = firstNumber + secondNumber; //kunci bernilai hasil penjumlahan kedua angka
    break;
  case 'x' : //jika operan adala x
    kunci = firstNumber * secondNumber; //kunci bernilai hasil perkalian kedua angka
    break;
  }
  lcd.setCursor(0, 1); //set posisi kursor ke baris 2
  if (ans==kunci){ //jawaban sama dengan kunci
    isRight = true; //jawaban benar
  }
  else{ //jawaban tidak sama dengan kunci
    isRight = false; //jawaban salah
  } 
}

char getKey(){ //fungsi untuk menangkap dan mengembalikan nilai tombol yang ditekan
  char key = 0; // 0 menandakan tidak ada tombol yang ditekan
  for(int column = 0; column < numCols; column++) //traversal dari kolom 0 ke kolom terakhir
  {
    digitalWrite(colPins[column],LOW); //aktivasi kolom
    for(int row = 0; row < numRows; row++) // memeriksa semua baris jika ada tombol yang ditekan
    {
      if(digitalRead(rowPins[row]) == LOW) // jika ada tombol yang ditekan
      {  
        delay(debounceTime); // debounce
        while(digitalRead(rowPins[row]) == LOW); // menunggu tombol dilepas
        key = keymap[row][column]; // menyimpan lokasi tombol yang ditekan
      }
    }
    digitalWrite(colPins[column],HIGH); // deaktivasi kolom
  }
  return key;  // mengembalikan nilai tombol yang ditekan, mengembalikan 0 jika tidak ada tombol yg ditekan
}
