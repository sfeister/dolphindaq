# Helper functions for various Sidekick Model 3 tasks

import numpy as np

def condition_pulse(pulse):
    """ Check pulse array readiness for writing Sidekick Model 3 LilLaser, and cast to uint8 (Helper function)
    """

    # Data validity checks
    assert(isinstance(pulse, np.ndarray))
    assert(np.ndim(pulse) == 1)
    assert(len(pulse) == 100)
    assert(np.max(pulse) <= 255)
    assert(np.min(pulse) >= 0)
    if (np.count_nonzero(pulse - pulse.round() > 0)):
        raise Exception("Pulse array contains fractional values (should be only integers, from 0 to 255).")

    # Data type conversion to uint8
    pulse_uint8 = np.uint8(pulse)

    return pulse_uint8