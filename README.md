# Laporan Penjelasan Final Project SISOP 2019

## Contents :
- [SOAL](#soal)
- [FUSE](#fuse)
- [MP3 Player](#mp3_player)

## SOAL <a name="soal"></a>
Buatlah sebuah music player dengan bahasa C yang memiliki fitur play nama_lagu, pause, next, prev, list lagu. Selain music player juga terdapat FUSE untuk mengumpulkan semua jenis file yang berekstensi .mp3 kedalam FUSE yang tersebar pada direktori /home/user. Ketika FUSE dijalankan, direktori hasil FUSE hanya berisi file .mp3 tanpa ada direktori lain di dalamnya. Asal file tersebut bisa tersebar dari berbagai folder dan subfolder. program mp3 mengarah ke FUSE untuk memutar musik.
Note: playlist bisa banyak, link [mp3player](http://hzqtc.github.io/2012/05/play-mp3-with-libmpg123-and-libao.html)

## FUSE <a name="fuse"></a>
Buat FUSE dengan root directory `/home/[user]`, yang hanya menampilkan file dengan ekstensi mp3.

### Jawab :
- buat fungsi init yang memindahkan seluruh file berekstensi .mp3 ke root directory.
- simpan nama file-file yang dipindahkan.
- hapus file-file yang telah dipindahkan pada fungsi destroy.

## MP3 Player <a name="mp3_player"></a>
Buat program c yang dapat berfungsi sebagai MP3 Player, yang memiliki fitur play list, play/pause, dan next/prev.

### Jawab :
- Buat thread untuk mengatasi interaksi user.
- Buat thread untuk mengatasi file mp3.
- Buat thread untuk menampilkan play list.