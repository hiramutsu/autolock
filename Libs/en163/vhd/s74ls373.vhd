LIBRARY synth;
USE synth.stdsynth.all;

ENTITY s74ls373 IS PORT (
  SIGNAL g, ocn, d8, d7, d6, d5, d4, d3, d2, d1: IN std_logic;
  SIGNAL q8,q7,q6,q5,q4,q3,q2,q1: OUT std_logic);
END s74ls373;

ARCHITECTURE general OF s74ls373 IS
  SIGNAL qint8,qint7,qint6,qint5,qint4,qint3,qint2,qint1: std_logic;
BEGIN
                  
-- Transparent latch of internal variables

PROCESS (g, d8, d7, d6, d5, d4, d3, d2, d1)
BEGIN
      IF (g = '1') THEN
         qint8 <= d8 after 12ns;
         qint7 <= d7 after 12ns;
         qint6 <= d6 after 12ns;
         qint5 <= d5 after 12ns;
         qint4 <= d4 after 12ns;
         qint3 <= d3 after 12ns;
         qint2 <= d2 after 12ns;
         qint1 <= d1 after 12ns;
         END IF;
END PROCESS;

-- Tristate section

q8 <= qint8 after 14ns WHEN (ocn = '0') ELSE 'Z' after 14ns ;

q7 <= qint7 after 14ns WHEN (ocn = '0') ELSE 'Z' after 14ns;
q6 <= qint6 after 14ns WHEN (ocn = '0') ELSE 'Z' after 14ns;
q5 <= qint5 after 14ns WHEN (ocn = '0') ELSE 'Z' after 14ns;
q4 <= qint4 after 14ns WHEN (ocn = '0') ELSE 'Z' after 14ns;
q3 <= qint3 after 14ns WHEN (ocn = '0') ELSE 'Z' after 14ns;
q2 <= qint2 after 14ns WHEN (ocn = '0') ELSE 'Z' after 14ns;
q1 <= qint1 after 14ns WHEN (ocn = '0') ELSE 'Z' after 14ns;

END general;


