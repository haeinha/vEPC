//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "corenetwork/binder/PgwBinder2.h"
#include "corenetwork/deployer/LteDeployer.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include <cctype>
#include "corenetwork/nodes/InternetMux.h"
#include "corenetwork/binder/LteBinder.h"
#include "corenetwork/binder/UeSet.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"
#include <iostream>
#include <string>
using namespace std;

Define_Module(PgwBinder2);



L3Address PgwBinder2::getPgwAddress(){
    //Enter_Method("getPgwAddress");
    L3Address pgwAddress_;
    pgwAddress_ = L3AddressResolver().resolve("pgw");
    return pgwAddress_;
}

L3Address PgwBinder2::assignPgwAddress(){
    L3Address pgwAddress_;
    int i;
    i=findPGWwithMinUE();
    const char* name=pgwlist[i]->getPGWname();
    pgwlist[i]->numUE++;

    pgwAddress_=L3AddressResolver().resolve(name);

    return pgwAddress_;
}
L3Address PgwBinder2::assignPgwAddress(IPv4Datagram *  address){
    L3Address pgwAddress_;
    IPv4Address &srcAddress = address->getSrcAddress();
    int i;
    i=findPGWwithMinUE();
    const char* name=pgwlist[i]->getPGWname();
    pgwlist[i]->numUE++;
    MacNodeId MacId=binder->getMacNodeId(srcAddress);
    const char* uename=binder->getModuleNameByMacNodeId(MacId);
    ueset* UE=check_and_cast<ueset*>(getSimulation()->getModuleByPath(uename)->getSubmodule("UeSet"));


    pgwAddress_=L3AddressResolver().resolve(name);
    pgwlist[i]->setsrcUE(MacId);
    UE->setpgwaddress(pgwAddress_);
    return pgwAddress_;
}
L3Address PgwBinder2::assignPgwAddress(IPv4Datagram *  address,eNBset *eNBsetptr){

    //HAEIN: assigningPgwAddress as the follow up sequence 1. select DC 2. assign PGW.
      L3Address pgwAddress_;
      IPv4Address &srcAddress = address->getSrcAddress();
      int DCindex; //selected DC index
      int PGWindex;//selected PGW in selected DC index
      //select DC
      DCindex=selectDCbyeNB(eNBsetptr); //given eNBset *eNBsetptr have a pingvector. select DC by ping info.
      //select PGW
      PGWindex=findPGWwithMinUEinDC(DClist[DCindex]);

      const char* name=DClist[DCindex]->pgwlist[PGWindex]->getPGWname();
      DClist[DCindex]->pgwlist[PGWindex]->numUE++;
      DClist[DCindex]->numUE++;
      MacNodeId MacId=binder->getMacNodeId(srcAddress);
      const char* uename=binder->getModuleNameByMacNodeId(MacId);
      ueset* UE=check_and_cast<ueset*>(getSimulation()->getModuleByPath(uename)->getSubmodule("UeSet"));


      pgwAddress_=L3AddressResolver().resolve(name);
      DClist[DCindex]->pgwlist[PGWindex]->setsrcUE(MacId);
      DClist[DCindex]->setsrcUE(MacId);
      UE->setpgwaddress(pgwAddress_);

      EV<<"UE NAME: "<<uename<<"is connected to DC name: "<<DClist[DCindex]->getDCname()<<" PGWname: "<<name;
      return pgwAddress_;
}
void PgwBinder2::registerDC(char* name){

    DClist[numDC++]=new DC(name);
}
void PgwBinder2::registerPGW(char* name)
{
    //find the DC in which PGW is involved and register it to DC.
    char* temp;
    std::string PGW=name;
    //pgwlist[numPGW++]=new Pgw(name); //original code
    temp=new char[5];

    for(int i=0;i<numDC;i++){
    strcpy(temp,(char*)DClist[i]->getDCname());
    if(PGW.find(temp)!=string::npos) //if pgwname has DCname
    {
        EV<<"register "<<name<<"to "<<DClist[i]->getDCname()<<endl;
        DClist[i]->registerPGW(name);
    }
    }
}
int PgwBinder2::findPGWwithMinUE()
{
    int temp;
    int min;
    int i;
    int minindex;
    for(i=0;i<numPGW;i++)
    {
        if(i==0)
            {
            temp=pgwlist[i]->numUE;
            min=temp;
            minindex=i;

            }
        else
            {
            temp=pgwlist[i]->numUE;
            if(temp<min){
                min=temp;
                minindex=i;
            }

            }
    }
    return minindex;
}
int PgwBinder2::findPGWwithMinUEinDC(DC* DCinstance){

    //in here implement the PFW selection algorithm
        int temp;
       int min;
       int minindex;
       for(int i=0;i<DCinstance->numPGW;i++)
       {
           if(i==0)
               {
               temp=DCinstance->pgwlist[i]->numUE;
               min=temp;
               minindex=i;


               }
           else
               {
               temp=DCinstance->pgwlist[i]->numUE;
               if(temp<min){
                   min=temp;
                   minindex=i;
               }

               }

       }
       EV<<"min UE is index: "<<minindex;
       return minindex;
}
int PgwBinder2:: DCalgorithm( std::vector<std::pair<pingpair,DC*>> DCvector){
    double minfactor;
    double tempfactor;
    int selectedindex;
    EV<<"the DCvectorsize: "<<DCvector.size()<<endl;
    for(int i=0;i<(signed int)DCvector.size();i++){
        EV<<DCvector[i].first.first<<"is in for loop"<<endl;
        EV<<"The number of UE in "<<DCvector[i].first.first<<" is : "<<DCvector[i].second->numUE<<endl;
           if(i==0){
           //tempfactor=DCvector[i].first.second.dbl()*100000+DCvector[i].second->numUE;
               tempfactor=DCvector[i].second->numUE;
           minfactor=tempfactor;
           selectedindex=i;
           }
           else{
               //tempfactor=DCvector[i].first.second.dbl()*100000+DCvector[i].second->numUE;
               tempfactor=DCvector[i].second->numUE;
               if(tempfactor<minfactor)
                   {
                   minfactor=tempfactor;
                   selectedindex=i;
                   }
           }

       }
    EV<<"the return value is :"<<selectedindex<<endl;
    return selectedindex;
}
int PgwBinder2::selectDCbyeNB(eNBset* eNBsetptr){
    //in here implement the DC selection algoritm.
    double RTT=par(RTT);//RTT setting later it would be easier to configure with par(RTT)
    int index;
    int selectedindex;

    //HAEIN: pingvec is sorted, so the smaller RTT ping is 0~itr.
    pingvector::iterator iter=eNBsetptr->findsmallerping(RTT);
    pingvector::iterator temp;


    std::pair <pingpair,DC*> DCpair;
    std::vector<std::pair<pingpair,DC*>> DCvector;

    //0~itr;
    for(temp=eNBsetptr->pingvec.begin();temp!=iter;++temp){
       index=findDCbystring((*temp).first);
       if(index<0)
           throw cRuntimeError("selecDCbyeNB: there is no such DC %s",(*temp).first);
       EV<<"the pair is "<<(*temp).first<<" "<<DClist[index]->getDCname()<<endl;
       DCpair=std::make_pair(*temp,DClist[index]);
       DCvector.push_back(DCpair);
    }

    //factor= DC ping*100000+DCnumUE.
    selectedindex=DCalgorithm(DCvector);

   return selectedindex;


}

int PgwBinder2::findDCbystring(std::string name){
    for(int i=0;i<numDC;i++){
        if(DClist[i]->dcstr==name)
            return i;
    }
    return -1;
}

void PgwBinder2::showingAssets()
{
    std::cout<<"DCname||#PGWs||#UEs||pgw||#UEs"<<endl;;
    for(int i=0;i<numDC;i++){
        std::cout<<DClist[i]->dc<<"   ||"<<DClist[i]->numPGW<<"    ||"<<DClist[i]->numUE;
        for(int j=0;j<DClist[i]->numPGW;j++){
            std::cout<<"   ||"<<DClist[i]->pgwlist[j]->PGW<<" ||"<<DClist[i]->pgwlist[j]->numUE;
        }
        std::cout<<endl;
    }
   /* string displaymsg;
       string temp1,temp2,temp3;

       for(int i=0;i<numDC;i++){
           temp1=DClist[i]->dc;
           temp2=itoa(DClist[i]->numPGW);
           temp3=itoa(DClist[i]->numUE);
           displaymsg+=temp1+"   ||"+temp2+"   ||"+temp3;
               for(int j=0;j<DClist[i]->numPGW;j++){

                   temp1=DClist[i]->pgwlist[j]->PGW;
                   temp2=itoa(DClist[i]->pgwlist[j]->numUE);
                   displaymsg+="   ||"+temp1+"   ||"+temp2;
               }
               displaymsg+="\n";
           }
       std::cout<<displaymsg;
       */
}
void PgwBinder2::refreshDisplay() const
{

   char buf[1000];
   int len=0;
  len=sprintf(buf,"DC    ||#P||#U||pgw||#U");
  for(int i=0;i<numDC;i++){
       len+=sprintf(buf+len,"\n%s ||%d   ||%d",DClist[i]->dc,DClist[i]->numPGW,DClist[i]->numUE);
       for(int j=0;j<DClist[i]->numPGW;j++){
           len+=sprintf(buf+len,"   ||%s   ||%d",DClist[i]->pgwlist[j]->PGW,DClist[i]->pgwlist[j]->numUE);
       }

   }
    getDisplayString().setTagArg("t", 0, buf);
}







