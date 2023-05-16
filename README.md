# BE RESEAU
## TPs BE Reseau - 3 MIC Aittaleb Mohamed Barnavon Jules-iana


## Contenu du dépôqui vous interesse
Ce dépôt inclunotre code source final pour mictcp 
  - README.md (ce fichier) 
  - tsock_texte et tsock_video : lanceurs pour les applications de test fournies. 
  - src/mictcp.c : code source que l'on a d�veloppé
  - configurer.sh : script shell qui vous demandera la version que vous voudrez utiliser et la tol�rance aux pertes que vous souhaitez pour la v3 et au-delas

## Choir la version
La méthode pour choisir la version que nous recommandons est d'utiliser le script `configurer.sh`, assurez vous de le rendre ex�cutable par la commande :
`chmod +x configurer.sh`. Il ne fait que modifier une variable `version` qui controle la version dans `src/mictcp.c`.  

Si non vous pouvez toujours remonter aux commits tagg�s avec le nom de la version que vous voulez.

## Compiler:
Si vous avez utiliséle script pour choisir la version, il n'est pas n�cessaire de recompiler, si non veuillez ex�cuter `make`

## Notre avancement
Nous arrivéà    d�veloppéune version qui marche de la v4, (i.e fiabilit� partielle et �tablissement de la connexion.

## Bug observé
Avant d'ex�cuter tsock_text, assurez vous de ne pas avoir ex�cutétsock_video sur votre machine auparavant, nous avons observéque ceci faisait planter
notre application. La seule solution que l on a trouvéest de simplement reboot la machine.

## Choix remarquables 
### Parametres de la fiabilite partielle
Pour implanter la fiabilite partielle, nous repris notre solution pour la fiabilite totale, et nous avons ajoute les elements suivants
Lorsque le client re�oit un acquitement, il ajoute un 1 au buffer circulaire de son socket et incr�ment le num�ro de s�quence, si non, il calcule la proportion de 1 dans
ce buffer, et si elle est sup�rieure �au pourcentage de pertes tol�r�es, il ajoute un 0 au buffer, et retourne au client la taille envoy�e
 mais n incremente pas le numero d acquitement, si non, il renvoie le packet et attend toujours l'acquittement.

Afin de permettre l'implantation de cette fiabilite partielle, nous avons juggéque seul celui qui envoie les packets (ici le client) est en
position de controler cette fonctionalit�. De ce fait, il n'y pas de n�gociation, simplement c est la source envoie, et qui controle le pourcentage de 
pertes dans ce qu elle a envoy�.
 
### La r�alisation des IP_recv
Si vous vous penchez sur notre code source, vous verrez que l'on a fait usage de thread supl�mentaire alors que nous ne sommes pas all�s jusqu'à   la v4.2.
La raison de ceci est que nous avons rencontrédes bugs assez troublant concernant les IP_recv, ces derniers adoptent un comportement diff�rent d une execution a l'autre.
Nous avons alors d�cidéde les traiter comme une source de perte en tant que tel, et nous les ex�cutons toujours dans un thread c�tépuit. C�tésource, vu que le 
process_received_pdu est lancé dans un thread a part, cela n'a pas �tén�cese. 

### Etablissement de la connexion
Pour l'etablissement de la connexion, notre protocole effectue les op�rations suivantes :

Lorsque le client c�tésource� fat un mic_tcp_connect, l'applicatio fixe le numero de sequence associe a son socket a 0, puis envoie un pdu syn a ce meme numero de sequence 0.
 A sa reception cote puit, la fonction process_receive_pdu l'applicatio fixe aussi son numero de sequence associe a son socket a 0, envoie un pdu syn ack ce numero de sequence ,
