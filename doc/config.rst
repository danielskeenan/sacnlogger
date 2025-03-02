Configuration File
==================

The configuration file is in `YAML <https://en.wikipedia.org/wiki/YAML>`_ format. An example is below:

.. code:: yaml

   universes:
     - 1
     - 2
   usePap: true

universes (required)
   List of universe numbers to monitor. Valid sACN universes are between 1 and 63999 inclusive.

usePap (optional)
   If ``true``, per-address-priority will be respected. Defaults to ``false``. This is an extension by ETC to allow more
   granular priorities and is likely to be present on any network with ETC control equipment. See
   `here <https://support.etcconnect.com/ETC/Networking/General/Difference_between_sACN_per-address_and_per-port_priority>`_
   for more information.
