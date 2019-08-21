"""
Per ogni file nella lista files viene creato un grafico.
Il file è in formato colonnare: 
->la prima colonna rappresenta la X;
->la seconda colonna rappresenta la Y;

names dizionario dei nomi dei file per ogni file
scientific abilita o no la notazione scientifica
points inserisce un'altra linea su quella già disegnata, utile per marcare i punti scelti su una linea interpolata
"""
def drawSingle(dir, files, names="" x_label="x", y_label="y", style="r--", label="", title="", scientific=False, points=False, p_style="g^", show=False):
    for file in files:
        data = sp.getListFromFile(path.join(dir, file))
        data["x"], data["y"] = (list(t) for t in zip(*sorted(zip(data["x"], data["y"]))))

        # red dashes, blue squares and green triangles
        plt.plot(data["x"], data["y"], style, label=label)  # , t, t ** 2, 'bs', t, t ** 3, 'g^')
        if points:
            plt.plot(data["x"], data["y"], p_style)
        
        plt.legend()
        plt.title(title)
        plt.xlabel(x_label)
        plt.ylabel(y_label)
        plt.ticklabel_format(useOffset=scientific)
        
        if names != "":
            name = names[file]
        else:
            name = file
        plt.savefig(path.join(dir, *["plot", name + ".svg"]))
        plt.clf()
        if show:
            plt.show()
        plt.close('all')

"""
Disegna più grafici che hanno stessa X e stessa scala Y.
name filename
Files è una lista di file di dati da disegnare, stesso formato di drawSingle
labels è una lista di stringhe che rappresentano le label dei file
styles è una lista di stringhe che rappresentano lo stile delle linee dei relativi file
"""
def drawDouble(dir, files, name="", title="", x_label="x", y_label="y", styles=["r--","b--"], labels=["",""], show=False):

    i = 0
    for file in files:
        data = sp.getListFromFile(path.join(dir, file))
        data["x"], data["y"] = (list(t) for t in zip(*sorted(zip(data["x"], data["y"]))))
        
        plt.plot(data["x"], data["y"], styles[i], label=labels[i])
        
        i += 1

    plt.title(title)
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.legend()
    if name == "":
        name = files[0]+";double"
    plt.savefig(path.join(dir, *["plot", name + ".svg"]))
    plt.clf()
    if show:
        plt.show()
    plt.close('all')

"""
Disegna due grafici sovrapposti, ovvero che hanno la stessa X con y diverse, una a sx ed una a dx
Files è una lista di due file
"""
def drawOverlayed(dir, files, name="", title="", x_label="x", y_label1="y1", y_label2="y2", style1="r--", style2="b--"):
    for file in files:
        data = sp.getListFromFile(path.join(dir, file), z=True)
        tmp = data["x"]
        data["x"], data["y"] = (list(t) for t in zip(*sorted(zip(data["x"], data["y"]))))
        tmp, data["z"] = (list(t) for t in zip(*sorted(zip(tmp, data["z"]))))

        fig, ax1 = plt.subplots()
        ax1.set_xlabel(x_label)
        ax1.set_ylabel(y_label1) #, color="tab:red"
        ax1.ticklabel_format(useOffset=False)
        ax1.plot(data["x"], data["y"], style1)
        ax1.tick_params(axis='y') #, labelcolor="tab:red"
        
        ax2 = ax1.twinx()
        ax2.set_ylabel(y_label2)  #, color="tab:blue"
        ax2.ticklabel_format(useOffset=False)
        ax2.plot(data["x"], data["z"], style2)
        ax2.tick_params(axis='y') #, labelcolor="tab:blue"
        fig.tight_layout()
        
        """
        if "mul_001" in file:
            ax1.set_ylim([0.0,0.04])
            ax2.set_ylim([0,3])
        if "mul_10" in file:
            ax1.set_ylim([0,3.5])
            ax2.set_ylim([0,200])
        """
            
        plt.title(title)
        if name == "":
            name = files[0]+"-"+files[1]+";overlayed"
        plt.savefig(path.join(dir, *["plot", name + ".svg"]))
        plt.clf()
        plt.close('all')

"""
Disegna grafico a barre con i dati in filesBar e sopra una linea di un valore preso in value 
##Potrebbe non funzionare piu per via delle modifiche, controlla##
"""
def drawHistOverlayed(dir, y1, filesBar, name="", title="", label_y1="", label_bar="", x_label="x", y_label="y", style_y1="r--"):
    #plt.style.use('seaborn-deep')
    for file in filesBar:
        data2 = sp.getListFromFile(path.join(dir, file))

        data2["x"], data2["y"] = (list(t) for t in zip(*sorted(zip(data2["x"], data2["y"]))))
        
        #mylist = list(dict.fromkeys(data2["y"]))
        plt.bar([i for i in range(1, max(data2["x"])+1], data2["y"], align="center", width=0.35, label=label_bar)
        plt.plot([1,max(data2["x"])], [y1,y1], style_y1, label=label_y)
        plt.ylabel(y_label)
        plt.xlabel(x_label)
        plt.title(title)
        plt.legend(loc='upper right')
        if name == "":
            name = file+";hist"
        plt.savefig(path.join(dir, *["plot", name + ".svg"]))
        plt.clf()
        plt.close('all')

"""
Come drawDouble ma con più curve
@param files: files di dati colonnari (x-y), la x deve essere la stessa per ogni file
@param name: nome da dare al file.svg del grafico
@param x_label: label delle ascisse
@param y_label: label delle oridnate
@param start: non usate
@param end: non usate
@param patterns: per differenziare i vari file (es. files=[dropped.sp, response.sp] allora patterns=[drop,resp])
@param styles: stili dei diversi patterns
@param labels: nome della curva
@param preset: alcuni preset tipici
"""
def drawMore(dir, files, name, x_label="", y_label="", start="", end="", patterns=[], styles=[], labels=[] preset="bdTime"):
    for file in files:
        data = sp.getListFromFile(path.join(dir, file))
        data["x"], data["y"] = (list(t) for t in zip(*sorted(zip(data["x"], data["y"]))))
        
        if len(patterns) != 0:
            for i in range(len(patterns)):
                if patterns[i] in file:
                    plt.plot(data["x"], data["y"], styles[i], label=labels[i])
                    break;
        
        elif preset=="bdTime":
            if "service" in file:
                plt.plot(data["x"], data["y"], 'r--', label="serviceTime")
            if "queuing" in file:
                plt.plot(data["x"], data["y"], 'b--', label="queuingTime")
            if "balancer" in file:
                plt.plot(data["x"], data["y"], 'g--', label="balancerTime")  # , t, t ** 2, 'bs', t, t ** 3, 'g^')
            if "response" in file:
                plt.plot(data["x"], data["y"], 'k--', label="responseTime")
            
            """
            if "mul_001" in file:
                plt.ylim(0.0, 0.16)
            elif "mul_10" in file:
                plt.ylim(0.0, 16.0)
            """
        
        elif preset=="bdJobs":
            if "server[0]" in file:
                plt.plot(data["x"], data["y"], 'r--', label="droppedServerJobs")
            elif "dispatcher" in file:
                plt.plot(data["x"], data["y"], 'g--', label="droppedDispatcherJobs")
            elif "SLA" in file:
                plt.plot(data["x"], data["y"], 'b--', label="droppedJobsTotal")

            plt.ylim(0.0, 1.0)
                
        elif preset=="qlen":
            if "rho_0.2" in file:
                plt.plot(data["x"], data["y"], 'r--', label="rho 0.2")
            if "rho_0.5" in file:
                plt.plot(data["x"], data["y"], 'b--', label="rho 0.5")
            if "rho_0.8" in file:
                plt.plot(data["x"], data["y"], 'g--', label="rho 0.8")
            if "rho_0.9" in file:
                plt.plot(data["x"], data["y"], 'k--', label="rho 0.9")

    #plt.title(name)
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.legend()
    plt.savefig(path.join(dir, *["plot", name + ".svg"]))
    plt.clf()
