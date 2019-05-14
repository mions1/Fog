//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "FogDispatcherLoad.h"
#include "FogJob_m.h"
#include "FogPU.h"
#include <algorithm>
namespace fog {

Define_Module(FogDispatcherLoad);

void FogDispatcherLoad::initialize()
{
    probeSent = 0; //variabile usata per tener conto dal numero di messaggi probe mandati. Quando probeSent%fanout==0 vuol dire che sono arrivati tutti i messaggi di risposta
    incID = 0; //usato per assegnare l'ID ai job. if incID>9999, incID=0;
    jobSent = false;
    nJobsSent = 0; //tiene conto del numero di job spediti ad altri nodi.
    nJobsBlockedFromQueue = 0; //numero di job droppati.
    nJobs = 0; //numero di job ricevuti
    jobsTimedOut = 0; //numero job droppati per timeOut
    nJobsReceived = 0; //numero job ricevuti da altri nodi
    blockingProbability = 0; //calcolata come nJobsBlockedFromQueue / (nJobs + nJobsReceived)
    fanOut = par("fanOut");
    answerIterator = 0; //variabile usata come indice per scorrere il vettore delle risposte al probe
    FogPU *serv;
    trovato = false;
    ackCheck = false;
    serverFound = false;

    //Controllo quanti server ha il nodo
    for (servers = 0;; servers++)
    {
        if ((this->getParentModule()->getSubmodule("server", servers)) == NULL)
        {
            break;
        }

    }

    nApps = this->getParentModule()->par("nApps");
    serv = (FogPU *) this->getParentModule()->getSubmodule("server", 0);
    queueCapacity = (serv->capacity) * servers;

    serverBusy.resize(servers, 0); //vettore che tiene conto dello stato degli server 0-idle, 1-busy
    serverBusy.reserve(servers);
    queueBusy.resize(queueCapacity, 0); //tiene conto dello stato delle code di ogni server 0-posto libero, 1-posto occupato
    queueBusy.reserve(queueCapacity);
    probeGates.resize(nApps - 1); //vettore che contiene l'indice dei gate che drovranno ricevere i messaggi di probe da un certo nodo

    for (int i = 0; i < nApps - 1; i++)
        probeGates[i] = i;

    if (fanOut < nApps)
    {
        ackGates.resize(fanOut, -1); //vettore che contiene l'indice delle porte dei nodi che hanno risposto al messaggio di probe
        ackGates.reserve(fanOut);

        probeAnswer.resize(fanOut, 0); //contiene le risposte relative al messaggio di probe (-1 -> nack, incID -> ack , incID+10000 -> ack(posto libero nelle code)
        probeAnswer.reserve(fanOut);
    }
    else if (fanOut >= nApps)
    {
        fanOut = nApps - 1;
        ackGates.resize(nApps - 1, -1);
        ackGates.reserve(nApps - 1);

        probeAnswer.resize(nApps - 1, 0);
        probeAnswer.reserve(nApps - 1);
    }

}
FogDispatcherLoad::~FogDispatcherLoad()
{
    int OwnedSize = this->defaultListSize();
    for (int i = 0; i < OwnedSize; i++)
    {
        cOwnedObject *Del = (cOwnedObject *) this->defaultListGet(0);
        this->drop(Del);
        if ((strcmp(Del->getName(), "loadUpdate") == 0) || (strcmp(Del->getName(), "probeEvent") == 0) || (strcmp(Del->getName(), "ackEvent") == 0) || (strcmp(Del->getName(), "nackEvent") == 0)
                || (strcmp(Del->getName(), "job") == 0) || (strcmp(Del->getName(), "dropUpdate") == 0))
        {
            delete Del;
            Del = NULL;
        }
    }
}

void FogDispatcherLoad::finish()
{
    blockingProbability = nJobsBlockedFromQueue / (nJobs + nJobsReceived);
    if (blockingProbability > 1)
        blockingProbability = 1;

    nJobsDropped = ((jobsTimedOut + nJobsBlockedFromQueue) / nJobs) * 100;
    if(nJobsDropped>100)
        nJobsDropped=100;
    recordScalar("nJobsDropped", nJobsDropped);
    recordScalar("nJobsTimedOut", jobsTimedOut);
    recordScalar("nJobs", nJobs);
    recordScalar("nJobsBlockedFromQueue", nJobsBlockedFromQueue);
    recordScalar("nJobsSent", nJobsSent);
    recordScalar("nJobsReceived", nJobsReceived);
    recordScalar("blockingProbability", blockingProbability);

}

/**
 * Gestisce i job che arrivano dalla sorgente o da altri nodi, decide a quale PU farlo processare, se fare probing o se droppare
 *
 * Ricezione di un job:
 *      Se il job proviene da un altro nodo
 *          -> Se ho pieni sia server che code lo droppo
 *      Se il job proviene da locale
 *          -> Se ho un server libero lo processo
 *          -> Se ho i server pieni faccio probing
 *
 * Ricezione di un probe:
 *      ->Se ho server libero mando un ACK
 *      ->Altrimenti, se ho code libere mando ACK+1000
 *      ->Altrimenti, se ho tutto pieno, mando NACK
 *
 * Ricezione ACK:
 *      ->Se ho ACK < 1000 allora qualcuno ha server liberi e lo mando a lui
 *      ->Se ho ACK > 1000 controllo se ho posti in coda locale, se si lo tengo altrimenti lo mando fuori
 *      ->Se ho solo NACK controllo le mie code, se son piene droppo
 *
 */
void FogDispatcherLoad::handleMessage(cMessage *msg)
{
    if (strcmp(msg->getName(), "job") == 0)
    {

        serverFound = false;

        cGate *temp = msg->getArrivalGate();
        //se il job proviene da un altro nodo controllo se ho un server libero. se no controllo le code. se �� tutto pieno il job verr�� mandato al primo server e poi droppato.
        int strcompare = strcmp(temp->getName(), "outerLoad");
        if (strcompare == 0)
        {
            for (unsigned int iterator = 0; iterator < serverBusy.size(); iterator++)
            {
                if (serverBusy[iterator] == 0)
                {
                    nJobsReceived++;
                    send(msg, "out", iterator);
                    serverBusy[iterator] = 1;
                    serverFound = true;
                    break;
                }
                else
                    continue;
            }
            if (!serverFound) //metto nelle queue
            {
                bool queueFound = false;
                int whichServer = 0;
                int temp = 0;
                for (unsigned int iterator = 0; iterator < queueBusy.size(); iterator++)
                {
                    if (temp >= (queueCapacity / servers))
                    {
                        whichServer++;
                        temp = 1;
                    }
                    else
                        temp++;
                    if (queueBusy[iterator] == 0)
                    {
                        nJobsReceived++;
                        queueFound = true;
                        queueBusy[iterator] = 1;
                        send(msg, "out", whichServer);
                        break;
                    }
                }
                if (!queueFound)
                {

                    send(msg, "out", 0);

                }
            }
            //reset queue and servers if ack was sent but job wasnt received

        }
        //se il job proviene dal source controllo lo stato dei server. teniamo conto del caso con fanout=0 e scenari con un solo nodo.
        else
        {
            for (unsigned int iterator = 0; iterator < serverBusy.size(); iterator++)
            {
                if (serverBusy[iterator] == 0)
                {
                    nJobs++;
                    send(msg, "out", iterator);
                    serverBusy[iterator] = 1;
                    serverFound = true;
                    break;
                }
                else
                    continue;
            }
            if (!serverFound)
            {
                if ((this->gateSize("probeOuterLoad")) == 0 || fanOut == 0)
                {
                    trovato = false;
                    int temp = 0;
                    int whichServer = 0;
                    for (unsigned int iterator = 0; iterator < queueBusy.size(); iterator++) //2
                    {
                        if (temp >= (queueCapacity / servers))
                        {
                            whichServer++;
                            temp = 1;
                        }
                        else
                            temp++;
                        if (queueBusy[iterator] == 0)
                        {
                            queueBusy[iterator] = 1;
                            trovato = true;
                            jobSent = true;
                            nJobs++;
                            send(msg, "out", whichServer);
                            break;
                        }
                        else
                            continue;
                    }
                    if (!trovato)
                    {

                        send(msg, "out", 0);

                    }
                }
                //se tutti i server locali sono pieni proseguiamo con l'invio di fanOut messaggi di probe. gli indici delle porte sono contenute
                //nel vettore probeGates il quale viene mischiato con la funzione random_shuffle
                else
                {
                    FogJob *jobCopy = (FogJob *) msg;
                    jobCopy->setId(incID);

                    incID++;
                    if (incID > 9999)
                        incID = 0;
                    buffer.insert(jobCopy);
                    if (fanOut < nApps)
                    {
                        std::random_shuffle(probeGates.begin(), probeGates.end());

                        for (int jCheck = 0; jCheck < fanOut; jCheck++)
                        {
                            probeEvent = new cMessage("probeEvent", jobCopy->getId());
                            probeSent++;
                            send(probeEvent, "probeOuterLoad", probeGates[jCheck]);
                        }
                    }

                    else
                    {
                        for (int jCheck = 0; jCheck < nApps - 1; jCheck++)
                        {
                            probeEvent = new cMessage("probeEvent", jobCopy->getId());
                            probeSent++;
                            send(probeEvent, "probeOuterLoad", jCheck);
                        }
                    }
                }
            }
        }
    }

    else if (strcmp(msg->getName(), "probeEvent") == 0)
    {
        bool check = false;
        //al ricevimento di un messaggio di probe controllo se ci sono server IDLE e mando ACK in caso di esito positivo.

        for (unsigned int iterator = 0; iterator < serverBusy.size(); iterator++)
        {
            if (serverBusy[iterator] == 0)
            {

                cGate * sender = msg->getSenderGate();
                for (cModule::GateIterator i(this); !i.end(); i++)
                {
                    cGate *gate = *i;
                    std::string gateStr = gate->getName();
                    if (gateStr == "probeOuterLoad" && gate->getPathEndGate()->getOwnerModule() == sender->getOwnerModule())
                    {
                        int senderId = gate->getIndex();
                        ackEvent = new cMessage("ackEvent", msg->getKind());
                        send(ackEvent, "probeOuterLoad", senderId);
                        check = true;

                    }
                }
            }
            else
                continue;
            if (check)
                break;
        }
        if (!check)
        {
            //se non ci sono server controllo le code (se disponibili). Se troviamo un posto nelle code, incrementiamo il kind del messaggio di probe di 10000
            bool checkQueue = false;
            for (unsigned int iterator = 0; iterator < queueBusy.size(); iterator++)
            {

                if (queueBusy[iterator] == 0)
                {
                    checkQueue = true;
                    break;
                }
            }
            if (checkQueue)
            {
                msg->setKind(msg->getKind() + 10000);

                cGate * sender = msg->getSenderGate();
                for (cModule::GateIterator i(this); !i.end(); i++)
                {
                    cGate *gate = *i;
                    std::string gateStr = gate->getName();
                    if (gateStr == "probeOuterLoad" && gate->getPathEndGate()->getOwnerModule() == sender->getOwnerModule())
                    {

                        int senderId = gate->getIndex();
                        ackEvent = new cMessage("ackEvent", msg->getKind());
                        send(ackEvent, "probeOuterLoad", senderId);

                    }
                }
            }
            //se le code e i server sono occupati mandiamo un nack
            else
            {
                cGate * sender = msg->getSenderGate();
                for (cModule::GateIterator i(this); !i.end(); i++)
                {
                    cGate *gate = *i;
                    std::string gateStr = gate->getName();
                    if (gateStr == "probeOuterLoad" && gate->getPathEndGate()->getOwnerModule() == sender->getOwnerModule())
                    {

                        int senderId = gate->getIndex();
                        nackEvent = new cMessage("nackEvent", msg->getKind());
                        send(nackEvent, "probeOuterLoad", senderId);
                    }
                }
            }
        }
        delete msg;
    } //fine probeEvent

    else if (strcmp(msg->getName(), "ackEvent") == 0)
    {
        cGate *temp = msg->getArrivalGate();
        probeAnswer[answerIterator] = msg->getKind(); //registriamo le risposte dei messaggi di probe

        ackCheck = true;

        probeSent--;
        ackGates[answerIterator] = temp->getIndex(); //registriamo l'indice dei gate che hanno risposto al messaggio di probe
        answerIterator++;
        if (probeSent % fanOut == 0)
        {
            answerIterator = 0;

            for (std::vector<int>::iterator it = probeAnswer.begin(); it != probeAnswer.end(); ++it)
                EV << ' ' << *it;
            EV << endl;
            //controlliamo le risposte. se probeAnswer<10000 vuol dire che ci sono server liberi in altri nodi
            if (!jobSent)
            {
                for (unsigned int k = 0; k < probeAnswer.size(); k++)
                {

                    if (probeAnswer[k] < 10000 && probeAnswer[k] != -1)
                    {
                        if (!buffer.isEmpty())
                        {

                            for (int j = 0;; j++)
                            {

                                FogJob *job;
                                job = (FogJob *) buffer.get(j);
                                //in caso ci siano pi�� job nel buffer del dispatcher cerchiamo il job con ID uguale a quello del messaggio di probe
                                if (job->getId() == probeAnswer[k])
                                {

                                    buffer.remove(buffer.get(j));
                                    send(job, "probeOuterLoad", ackGates[k]);
                                    trovato = true;
                                    ackCheck = false;
                                    jobSent = true;
                                    nJobsSent++;
                                    break;

                                }
                                else
                                    continue;

                            }
                        }
                        else
                            break;
                        if (trovato)
                        {

                            trovato = false;
                            break;
                        }

                    }
                    else
                        continue;
                }
            }
            if (!jobSent)
            {
                //se non abbiamo trovato server libero in altri nodi controlliamo se per caso ci sono dei posti nelle code.
                for (unsigned int k = 0; k < probeAnswer.size(); k++) //3
                {
                    //se abbiamo trovato un posto in una coda di un nodo esterno, controlliamo prima lo stato delle code locali
                    if (probeAnswer[k] >= 10000 && probeAnswer[k] != -1 && !buffer.isEmpty())
                    {
                        int whichServer = 0;
                        int temp = 0;
                        for (unsigned int iterator = 0; iterator < queueBusy.size(); iterator++) //2
                        {

                            if (temp >= (queueCapacity / servers))
                            {
                                whichServer++;
                                temp = 1;
                            }
                            else
                                temp++;
                            if (queueBusy[iterator] == 0)
                            {
                                queueBusy[iterator] = 1;
                                for (int j = 0;; j++) //1
                                {
                                    FogJob *job;
                                    job = (FogJob *) buffer.get(j);
                                    EV << job->getId() << "--job ID" << endl;

                                    if ((job->getId()) + 10000 == probeAnswer[k])
                                    {
                                        job->setKind(job->getId() + 10000);
                                        buffer.remove(buffer.get(j));
                                        trovato = true;
                                        ackCheck = false;
                                        jobSent = true;
                                        nJobs++;
                                        send(job, "out", whichServer);
                                        break;
                                    }
                                    else
                                        continue;
                                }
                                if (trovato)
                                    break;
                            }
                            else
                                continue;
                        }
                        if (trovato)
                        {
                            trovato = false;
                            break;
                        }
                        //se non ci sono posti nelle code locali mandiamo il job al primo nodo che ha risposto con un ack
                        else if (!buffer.isEmpty())
                        {

                            for (int j = 0;; j++)
                            {

                                FogJob *job;
                                job = (FogJob *) buffer.get(j);

                                EV << job->getId() << "--job ID" << endl;

                                if ((job->getId()) + 10000 == probeAnswer[k])
                                {
                                    job->setKind(job->getId() + 10000);
                                    buffer.remove(buffer.get(j));
                                    trovato = true;
                                    ackCheck = false;
                                    jobSent = true;
                                    nJobsSent++;
                                    send(job, "probeOuterLoad", ackGates[k]);
                                    break;
                                }
                                else
                                    continue;
                            }
                        }
                        if (trovato)
                        {
                            trovato = false;
                            break;
                        }
                    }
                    else
                        continue;
                }
            }

        }
        jobSent = false;
        delete msg;
    } //fine ack

    //registriamo il nack ricevuto
    else if (strcmp(msg->getName(), "nackEvent") == 0)
    {
        cGate *temp = msg->getArrivalGate();
        probeAnswer[answerIterator] = -1;

        probeSent--;

        ackGates[answerIterator] = temp->getIndex();
        answerIterator++;
        //se abbiamo ricevuto solo degli nack controlliamo se abbiamo un posto nelle code proprie altrimenti droppiamo il job.
        if (probeSent % fanOut == 0 && !ackCheck)
        {
            answerIterator = 0;
            int whichServer = 0;
            int temp = 0;

            for (std::vector<int>::iterator it = probeAnswer.begin(); it != probeAnswer.end(); ++it)
                EV << ' ' << *it;
            EV << endl;

            if (!buffer.isEmpty())
            {

                for (int j = 0;; j++)
                {
                    FogJob *job;
                    job = (FogJob *) buffer.get(j);
                    bool check = false;
                    if (job->getId() == msg->getKind())
                    {
                        for (unsigned int iterator = 0; iterator < queueBusy.size(); iterator++)
                        {
                            if (temp >= (queueCapacity / servers))
                            {
                                whichServer++;
                                temp = 1;
                            }
                            else
                                temp++;
                            if (queueBusy[iterator] == 0)
                            {
                                queueBusy[iterator] = 1;
                                check = true;
                                buffer.remove(buffer.get(j));
                                nJobs++;
                                send(job, "out", whichServer);
                                break;
                            }
                            else
                                continue;
                        }
                        if (!check)
                        {
                            buffer.remove(buffer.get(j));
                            send(job, "out", 0);
                            break;
                        }
                    }
                    if (check)
                        break;
                    else
                        continue;

                }

            }
        }
        //se tra i nack ricevuti ci sono degli ack controlliamo di che tipo sono
        //facciamo le stesse procedure di controllo dell'ack.
        else if (probeSent % fanOut == 0 && ackCheck)
        {
            nJobsSent++;
            answerIterator = 0;

            for (std::vector<int>::iterator it = probeAnswer.begin(); it != probeAnswer.end(); ++it)
                EV << ' ' << *it;
            EV << endl;

            if (!jobSent)
            {
                for (unsigned int k = 0; k < probeAnswer.size(); k++)
                {

                    if (probeAnswer[k] < 10000 && probeAnswer[k] != -1)
                    {
                        if (!buffer.isEmpty())
                        {
                            for (int j = 0;; j++)
                            {

                                FogJob *job;
                                job = (FogJob *) buffer.get(j);
                                if (job->getId() == probeAnswer[k])
                                {
                                    buffer.remove(buffer.get(j));
                                    nJobsSent++;
                                    send(job, "probeOuterLoad", ackGates[k]);
                                    trovato = true;
                                    ackCheck = false;
                                    jobSent = true;
                                    break;

                                }
                                else
                                    continue;

                            }
                        }
                        else
                            break;
                        if (trovato)
                        {

                            trovato = false;
                            break;
                        }

                    }
                    else
                        continue;
                }
            }
            if (!jobSent)
            {
                for (unsigned int k = 0; k < probeAnswer.size(); k++) //3
                {

                    if (probeAnswer[k] >= 10000 && probeAnswer[k] != -1 && !buffer.isEmpty())
                    {
                        int whichServer = 0;
                        int temp = 0;
                        for (unsigned int iterator = 0; iterator < queueBusy.size(); iterator++) //2
                        {

                            if (temp >= (queueCapacity / servers))
                            {
                                whichServer++;
                                temp = 1;
                            }
                            else
                                temp++;
                            if (queueBusy[iterator] == 0)
                            {
                                queueBusy[iterator] = 1;
                                for (int j = 0;; j++) //1
                                {

                                    FogJob *job;
                                    job = (FogJob *) buffer.get(j);
                                    if ((job->getId()) + 10000 == probeAnswer[k])
                                    {
                                        job->setKind(job->getId() + 10000);
                                        buffer.remove(buffer.get(j));
                                        trovato = true;
                                        ackCheck = false;
                                        jobSent = true;
                                        nJobs++;
                                        send(job, "out", whichServer);
                                        break;
                                    }
                                    else
                                        continue;
                                }
                                if (trovato)
                                    break;
                            }
                            else
                                continue;
                        }
                        if (trovato)
                        {
                            trovato = false;
                            break;
                        }
                        else if (!buffer.isEmpty())
                        {

                            for (int j = 0;; j++)
                            {

                                FogJob *job;
                                job = (FogJob *) buffer.get(j);
                                if ((job->getId()) + 10000 == probeAnswer[k])
                                {

                                    job->setKind(job->getId() + 10000);
                                    buffer.remove(buffer.get(j));
                                    trovato = true;
                                    ackCheck = false;
                                    jobSent = true;
                                    nJobsSent++;
                                    send(job, "probeOuterLoad", ackGates[k]);
                                    break;

                                }
                                else
                                    continue;

                            }

                        }
                        if (trovato)
                        {

                            trovato = false;
                            break;
                        }
                    }
                    else
                        continue;
                }
            }
        }
        jobSent = false;
        delete msg;
    }
    //messaggio di update proveniente dai server locali
    else if ((strcmp(msg->getName(), "loadUpdate") == 0) || (strcmp(msg->getName(), "dropUpdate") == 0))
    {
        cGate *tmp = msg->getArrivalGate();
        int idx = tmp->getIndex();
        bool isEmpty = true;
        int pos = 0;
        int temp = 0;
        //loadupdate con kind 0 indica la fine di un job. Controlliamo lo stato delle code del server che ha generato tale messaggio.
        //Se il vettore delle code ha solo degli zeri vuol dire che il server �� diventato IDLE. Aggiorniamo i relativi vettore a seconda del caso.
        if (msg->getKind() == 0)
        {
            temp = queueCapacity / servers;
            for (unsigned int queueIterator = (idx * (queueCapacity / servers)); queueIterator < queueBusy.size(); queueIterator++)
            {

                if (queueBusy[queueIterator] == 1)
                {
                    isEmpty = false;
                    pos = queueIterator;
                    break;
                }
                if (temp != 0)
                    temp--;
                if (temp == 0)
                    break;
            }
            if (!isEmpty)
            {
                queueBusy[pos] = 0;
            }
            else
                serverBusy[idx] = 0;
        }
        else if (msg->getKind() == 1)
            nJobsBlockedFromQueue++;
        else if (msg->getKind() == 2 || msg->getKind() == 3)
        {
            jobsTimedOut++;
            temp = queueCapacity / servers;
            for (unsigned int queueIterator = (idx * (queueCapacity / servers)); queueIterator < queueBusy.size(); queueIterator++)
            {

                if (queueBusy[queueIterator] == 1)
                {
                    isEmpty = false;
                    pos = queueIterator;
                    break;
                }
                if (temp != 0)
                    temp--;
                if (temp == 0)
                    break;
            }
            if (!isEmpty)
            {
                queueBusy[pos] = 0;
            }
            else
                serverBusy[idx] = 0;
        }
        delete msg;
    }

}
}
