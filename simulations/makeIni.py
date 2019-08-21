import sys

"""
Auto-generazione di file di configurazione .ini
Bisogna aggiungere solo qualche riga per cambiare impostazioni
"""


if __name__ == "__main__":
    fileName = sys.argv[1]	#file .ini (empty or to append)
    
    #Aggiungi qui un'altra lista per aggiungere combinazioni
    procTimeMul = ["0.01","1.0"]
    rho=["0.2","0.5","0.8","0.9"]
    thr=["1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20"]
    #thr=["1","5","10","15","20"]

    f = open(fileName,"a")

	#Aggiungi altri for, uno per ogni lista di sopra (per fare le combinazioni)
    for time in procTimeMul:
        for r in rho:
            for t in thr:
            	#Cambia qui se vuoi nomi di config diverse
                config = "\n[Config Fog-mul_"+time.replace(".","")+"-rho_"+r.replace(".","")+"-thr_"+t+"]"	#config name ex. [Config Fog-mul_10-rhp_02]
       
                config += "\nextends=FogBase"
                
                #Aggiungi qui le altre impostazioni se hai aggiunto liste
                config += "\nFogSequentialForward.proctimeMul = "+time
                config += "\nFogSequentialForward.rho = "+r
                config += "\nFogSequentialForward.fog[*].dispatcher.thr = "+t
                
                config += "\n"
                f.write(config)
