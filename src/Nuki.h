#pragma once
/*
 * nuki.h
 *
 *  Created on: 14 okt. 2020
 *      Author: Jeroen
 */

#include "BLEDevice.h"
#include "nukiConstants.h"
#include "Arduino.h"

class Nuki {
  public:
    Nuki();
    virtual ~Nuki();

    virtual void initialize();
    bool pairBle(std:string bleAddress);
    bool isConnected();

    bool executeLockAction(lockAction action);

  protected:

  private:
    
    
};