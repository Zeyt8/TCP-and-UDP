Server:
	Serverul trateaza mesajele in 4 moduri:
	1. Daca vin de pe socketul rezervat pentru conexiuni de la client.
		Socketul este un stream ce asculta connection requests. Accepta requestul, creeaza un socket nou pentru acel client si se pregateste sa primeasca un mesaj cu ID-ul clientului.
	2. Daca vin de pe socketul rezervat pentru mesaje de la UDP clients
		Mesajului i se adauga la final IPul sursei, portul sursei si '\r' pentru incadrare. Se cauta daca vreun client este abonat la acel topic. Daca clientul este conectat i se trimite mesajul, daca nu si sf este setat la 1 pentru acel topic se adauga la o coada de mesaje a acelui client.
	3. Daca vin de la consola
		Daca mesajul este "exit" se inchide serverul si se trimite mesaj la toti clientii sa se inchida.
	4. Altfel
		Daca este asteptat un mesaj de ID se citeste mesajul. Se cauta daca exista un alt client cu acelasi ID. Daca exista si nu este conectat, atunci se conecteaza si i se trimit toate mesajele pastrate in coada. Daca exista si este conectat, atunci nu se permite conectarea. Daca nu exista se creeaza un client nou ce se adauga la lista de clienti.
		Daca nu este asteptat un mesaj de ID, atunci se verifica daca mesajul este de tip subscribe sau unsubscribe. In cazul subscribe, se citeste topicul si sf si se adauga la clientul 	aferent. In cazul de unsubscribe se sterg topicul si sful necesare.
Subscriber
	Subscriberul trateaza mesaje in 2 moduri:
	1. Mesaje de la consola
		Subscribe trimite la server un mesaj de subscribe. Mesajul are primul byte 's' pentru identificare, urmat de 50 de bytes cu topicul, 1 byte cu valoarea lui sf si 1 byte de '\0'.
		Unsubscribe trimite la server un mesaj de unsubscribe. Mesajul are primul byte 'u', urmat de 50 de bytes cu topicul si 1 byte de '\0'.
		Exit inchide subscriberul.
	2. Mesaje de la server
		Daca socketul de primire este activ, subscriberul primeste date intr-un while pana cand un mesaj intreg a fost primit. Acest lucru se verifica prin prezenta caracterului '\r' ce marcheaza finalul unui mesaj. La primire de date, pointer se incrementeaza cu numarul de bytes primit, adica la pozitia unde sa se adauge in bufferul de citire. Cand un mesaj intreg este depistat, acesta este copiat in alt buffer, inceputul urmatorului mesaj este mutat la inceputul bufferului de citire, iar pointerul este decrementat cu lungimea unui mesaj.
		Se trece la prelucrarea mesajului. Se verifica tipul de mesaj. In cazul numeric se citeste numarul din buffer si se modifica in asa fel incat sa aiba ordinea corecta a bytilor. Se fac alte calcule in functie de caz si se afiseaza. IPul si portul sursei se iau de la finalul mesajului, pozitia lor in mesaj este explicata in Server(2.).
		
Implementarea pentru ue_vector este preluata de aici: https://codereview.stackexchange.com/questions/253173/generic-vector-implemented-in-c-language
List si queue sunt luate din scheletul temei 1.
