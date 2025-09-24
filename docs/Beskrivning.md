# gpedit
## Ett textbaserat program för att redigera gitarrtabulatur (GuitarPro3-filer)

<br>

### Mål
Ett program, skrivet i C++, som kan användas för grundläggande redigering av gitarrtabulatur i form av gp3-filer.\
Programet ska bestå av ett textbaserat gränssnitt, inspirerat av hur tabulatur vanligen skrivs i oformatterade textdokument (se nedan).

```
e|----------------------------------------------------------|
B|--------3------5------3-----------------------------------|
G|--2h3/4------------4-------------4\2p0--------------------|
D|------------------------------------------0---------0---0-|
A|-----------------------------------------------1/2--------|
E|----------------------------------------------------------|
```

<br>

### Vad behövs?
- Först och främst behövs det en klass som representerar den datastruktur som finns i en gp3-fil, med variabler för alla värden som kan förekomma.\
Klassen ska också ha funktioner för att läsa in en gp3-fil och spara datan som ett objekt, samt för att ta datan från ett sådant objekt och skriva till en gp3-fil.

- För själva redigeringen behövs funktioner för att visa datan i form av tabulatur, låta användaren navigera runt, samt att faktiskt redigera noterna.

- En gp3-fil kan ha flera spår, för att representera flera instrument. Därför behöver man också kunna välja vilket spår man redigerar, samt ta bort och skapa nya spår.

- Det behövs även något sätt att redigera filens metadata, såsom låttitel, artist, mm. Spåren har också egen metadata som ska gå att redigera.

- Programmet bör också innehålla funktionalitet för att skapa nya "tomma" filer. Detta borde ske automatiskt när användaren försöker öppna en fil som inte finns.


Längs vägen kommer det säkert att dyka upp andra saker som visar sig nödvändiga, men det här ger en grundläggande uppfattning om programmets struktur och funktion.

<br>

### Vad har gjorts hittills?
*2025-09-18*\
I nuläget är själva datastrukturen för filerna i princip klar, men kan eventuellt behöva finpoleras lite senare.\
Funktionerna för att läsa in en gp3-fil är nästintill klara.\
Nästa steg är att börja på funktionerna som sparar datan till en gp3-fil. När det är klart borde man kunna stoppa in en fil i programmet och få ut exakt samma fil.