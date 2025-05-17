	Acest proiect reprezinta o aplicatie client, care 
interactioneaza cu un API de tip REST. Aplicatia este o 
interfata in linia de comanda care permite comunicarea cu
o librarie de filme, utilizand protocolul HTTP.

	Am implementat functii pentru logare si deconectare admin
si utilizator normal. Sunt retinute cookieurile corespunzatoare
adminului si userului, daca este logat unul dintre ei, in variabile
globale. Cand este apelata functia logout_admin, este sters 
cookie-ul asociat adminului, analog si pentru user.

	Un admin poate doar sa vizualizeze utilizatori sau sa ii 
stearga.

	Un utilizator normal poate sa ceara acces la librarie. In urma
acestei cereri am salvat tokenul intr-o variabila globala, care este
golita in momentul deconectarii. Un utilizator poate sa vizualizeze,
sa adauge, sa modifice si sa stearga un film. De asemenea, poate
vedea, adauga si sterge o colectie, sau poate adauga sau elimina
filme din aceasta. In implementarea mea, toate aceste functii 
verifica existenta tokenului la inceput. Am utilizat o structura
User_info care retine id-ul colectiei si userul celui care o detine,
pentru a putea face verificarea in cazul operatiilor care pot fi
efectuate doar de owner.

	Am utilizat biblioteca Parson deoarece ofera functionalitatile
de parsare si construire obiecte JSON intr-un mod minimalist.
