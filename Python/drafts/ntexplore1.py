# Exploration of normative types

import numpy as np
import p4p

import time
from p4p import Value
from p4p.nt import NTScalar, NTNDArray
from p4p.server.thread import SharedPV

from datetime import datetime

class ScottNTNDArray(NTNDArray):
    """ Modified to allow for more customization """
    def wrap(self, value, uniqueId=0, dataTimeStamp=None, **kws):
        if isinstance(value, Value):
            return self._annotate(value, **kws)
        
        else:
            value = super().wrap(value, **kws)
            value['uniqueId'] = uniqueId
            
            if dataTimeStamp is not None: # this code section is copied from the NTBase class
                # dataTimeStamp may be: datetime, seconds as float or int, or tuple of (sec, ns)
                if isinstance(dataTimeStamp, datetime):
                    dataTimeStamp = datetime.timestamp(dataTimeStamp)

                if isinstance(dataTimeStamp, (int, float)):
                    sec, ns = divmod(dataTimeStamp, 1.0)
                    dataTimeStamp = (int(sec), int(ns*1e9))

                # at this point timestamp must be a tuple of (sec, ns)
                value['dataTimeStamp'] = {'secondsPastEpoch':dataTimeStamp[0], 'nanoseconds':dataTimeStamp[1]}
            
            return value

if __name__ == "__main__": 
    pv = SharedPV(nt=ScottNTNDArray(), initial=np.zeros(500))
    assert pv.current().raw['uniqueId'] == 0

    pv.post(np.ones(500), uniqueId=23, dataTimeStamp=time.time(), timestamp=time.time())
    assert pv.current().raw['uniqueId'] == 23
    
    
