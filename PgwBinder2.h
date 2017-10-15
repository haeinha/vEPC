//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef _LTE_LTEPGWBINDER2_H_
#define _LTE_LTEPGWBINDER2_H_

#include <omnetpp.h>
#include <string>
#include "common/LteCommon.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/common/L3Address.h"
#include "corenetwork/binder/Pgwinstance.h"
#include "corenetwork/binder/LteBinder.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "corenetwork/binder/eNBset.h"
#include "corenetwork/binder/DCset.h"
#include "corenetwork/binder/DCinstance.h"

using namespace inet;


typedef std::pair <std::string,simtime_t> pingpair;
typedef std::vector<std::pair<std::string,simtime_t>> pingvector;
class PgwBinder2 : public cSimpleModule
{
  private:
      Pgw* pgwlist[50]; // It would be better dynamically allocate by pgwnum
      DC *DClist[50]; // It would be better dynamically allocate by DCnum
      int numPGW;
      int numDC;
      LteBinder *binder;
      eNBset *eNBsetptr; //not yet used.
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stages){
    if(stages==INITSTAGE_PHYSICAL_ENVIRONMENT_2){
        binder=getBinder();
        showingAssets();
    }
    }

    virtual void handleMessage(cMessage *msg)
    {
    }

      virtual void refreshDisplay() const override;
  public:
      PgwBinder2(): numPGW(0),numDC(0){}
            ~PgwBinder2()
            {
                for(int i=0;i<numPGW;i++)
                {
                    delete pgwlist[i];
                }
            }

    L3Address getPgwAddress();
    L3Address assignPgwAddress();
    L3Address assignPgwAddress(IPv4Datagram *  address);
    L3Address assignPgwAddress(IPv4Datagram *  address,eNBset *eNBsetptr);
    void registerPGW(char* name);
    int findPGWwithMinUE();
    int findPGWwithMinUEinDC(DC* DCinstance);
    void registerDC(char* name);
    int selectDCbyeNB(eNBset* eNBsetptr);
    int findDCbystring(std::string name);
    int DCalgorithm( std::vector<std::pair<pingpair,DC*>> DCvector);
    void showingAssets();

};


#endif



