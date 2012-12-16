-- Pure VHDL version of xlabf using structural VHDL to put together
-- behavioral components.  Component blocks are listed first and the
-- top level entity, xlabf99 comes last.

LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY addr_gen IS PORT (
  SIGNAL amux_sel: IN std_logic;
  SIGNAL ext, char: IN std_logic_vector (5 DOWNTO 0);
  SIGNAL disp: IN std_logic_vector (2 DOWNTO 0);
  SIGNAL addr: OUT std_logic_vector (5 DOWNTO 0));
END addr_gen;


ARCHITECTURE xilinx OF addr_gen IS

-- Here is where your code for the address generator goes!

END xilinx;



LIBRARY ieee;

USE ieee.std_logic_1164.all;


-- The dff entity defines a simple D-flip flop for use as a component if you
-- Wish to use it.


ENTITY dff IS PORT (

  SIGNAL q: OUT std_logic;
  SIGNAL clk, d: IN std_logic);
END ENTITY dff;
ARCHITECTURE general OF dff IS
BEGIN -- Simple D-Flip Flop as a function call

PROCESS
BEGIN
        WAIT UNTIL (clk'EVENT AND clk  = '1');
	q <= d;
END PROCESS;

END general;

LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY fsm_ctrl IS PORT (
  SIGNAL a_new, clk, rst: IN std_logic;
  SIGNAL wsn, kenb, amux_sel: OUT std_logic);
END ENTITY fsm_ctrl;

ARCHITECTURE xilinx OF fsm_ctrl IS

-- Here is where your code for a finite state machine to control memory
-- writing and cathode enabling goes.


END xilinx;



LIBRARY ieee;

USE ieee.std_logic_1164.all;



ENTITY k_demux IS PORT (

  SIGNAL k_enb: IN std_logic;

  SIGNAL disp_k: IN std_logic_vector (2 DOWNTO 0);

  SIGNAL kout: OUT std_logic_vector (7 DOWNTO 0));
END k_demux;

ARCHITECTURE xilinx OF k_demux IS

-- Here is where your code for the demultiplexing of the display cathodes 
-- goes.



END xilinx;



-- VHDL code for a 24 bit counter using a modified carry look-ahead technique
-- for implementation in the FPGA

LIBRARY ieee;  -- Standard libraray for IEEE VHDL standard 1164 (1993)
USE ieee.std_logic_1164.all;

-- 4 bit block used as component in counter

ENTITY c4bit IS PORT (
	SIGNAL clk, ce: IN std_logic;
	SIGNAL BP: OUT std_logic;	-- Block Propagate
	SIGNAL Q: BUFFER std_logic_vector (3 DOWNTO 0));
END c4bit;

ARCHITECTURE xfpga OF c4bit IS
BEGIN
PROCESS 
BEGIN
	WAIT UNTIL (clk'EVENT AND clk = '1');
	Q(0) <= Q(0) XOR CE;
	Q(1) <= (Q(0) AND CE) XOR Q(1);
	Q(2) <= (Q(1) AND Q(0) AND CE) XOR Q(2);
	Q(3) <= (Q(2) AND Q(1) AND Q(0) AND CE) XOR Q(3);
END PROCESS;

	BP <= Q(0) AND Q(1) AND Q(2) AND Q(3);
END;

-- Now the code for the 24 bit counter itself.  First repeat library
-- usage for compatibility with FPGAExpress

LIBRARY ieee;  -- Standard libraray for IEEE VHDL standard 1164 (1993)
USE ieee.std_logic_1164.all;

ENTITY counter24 IS PORT (
	SIGNAL clk: IN std_logic;
	SIGNAL count: BUFFER std_logic_vector (23 DOWNTO 0);
	SIGNAL cout: OUT std_logic);
END;

ARCHITECTURE block_prop OF counter24 IS 
	-- Declare the port map for the c4bits 4 bit count block
COMPONENT c4bit 
	PORT (SIGNAL clk, ce: IN std_logic;
	SIGNAL BP: OUT std_logic;	-- Block Propagate
	SIGNAL Q: BUFFER std_logic_vector (3 DOWNTO 0));
END COMPONENT;

	SIGNAL carry: std_logic_vector (5 DOWNTO 1);
	SIGNAL bp: std_logic_vector (4 DOWNTO 1);
	SIGNAL dum: std_logic;
BEGIN
	dum <= '1';
cb0:  c4bit PORT MAP (clk, dum, carry(1), count(3 DOWNTO 0));
cb1:  c4bit PORT MAP (clk, carry(1), bp(1), count(7 DOWNTO 4));
cb2:  c4bit PORT MAP (clk, carry(2), bp(2), count(11 DOWNTO 8));
cb3:  c4bit PORT MAP (clk, carry(3), bp(3), count(15 DOWNTO 12));
cb4:  c4bit PORT MAP (clk, carry(4), bp(4), count(19 DOWNTO 16));
cb5:  c4bit PORT MAP (clk, carry(5), cout, count(23 DOWNTO 20));

carry(2) <= bp(1) AND carry(1);
carry(3) <= bp(2) AND bp(1) AND carry(1);
carry(4) <= bp(3) AND bp(2) AND bp(1) AND carry(1);
carry(5) <= bp(4) AND bp(3) AND bp(2) AND bp(1) AND carry(1);
END block_prop;


-- Now the structural description of interconnecting earlier blocks
-- This is the top-level design block!


LIBRARY ieee;  -- Standard libraray for IEEE VHDL standard 1164 (1993)

USE ieee.std_logic_1164.all;



ENTITY xlabf99 IS PORT (
	SIGNAL new_data, mclk: IN std_logic;
	SIGNAL char: IN std_logic_vector (5 DOWNTO 0);
	SIGNAL wr_n: OUT std_logic;
	SIGNAL addr: OUT std_logic_vector (5 DOWNTO 0);
	SIGNAL cathodes: OUT std_logic_vector (7 DOWNTO 0));
END xlabf99;

ARCHITECTURE xilinx OF xlabf99 IS
COMPONENT counter24 
	PORT (SIGNAL clk: IN std_logic;
	SIGNAL count: BUFFER std_logic_vector (23 DOWNTO 0);
	SIGNAL cout: OUT std_logic);
END COMPONENT;
COMPONENT k_demux
	PORT (SIGNAL k_enb: IN std_logic;
  	SIGNAL disp_k: IN std_logic_vector (2 DOWNTO 0);
  	SIGNAL kout: OUT std_logic_vector (7 DOWNTO 0));
END COMPONENT;
COMPONENT addr_gen
	PORT (SIGNAL amux_sel: IN std_logic;
  	SIGNAL ext, char: IN std_logic_vector (5 DOWNTO 0);
  	SIGNAL disp: IN std_logic_vector (2 DOWNTO 0);
  	SIGNAL addr: OUT std_logic_vector (5 DOWNTO 0));
END COMPONENT;
COMPONENT fsm_ctrl
	PORT (SIGNAL a_new, clk, rst: IN std_logic;
  	SIGNAL wsn, kenb, amux_sel: OUT std_logic);
END COMPONENT;

	SIGNAL count: std_logic_vector (23 DOWNTO 0);
	SIGNAL kenb, amux_sel, dum, gnd: std_logic;
BEGIN
	-- Wire up the three blocks


	gnd <= '0';	-- permanent low signal

U1:  counter24 PORT MAP (mclk, count (23 DOWNTO 0), dum);

U2:  addr_gen PORT MAP (amux_sel, count(23 DOWNTO 18), 
		char(5 DOWNTO 0), count(13 DOWNTO 11), addr(5 DOWNTO 0));
U3:  fsm_ctrl PORT MAP (new_data, mclk, gnd, wr_n, kenb, amux_sel);
U4:  k_demux  PORT MAP (kenb, count(13 DOWNTO 11), cathodes (7 DOWNTO 0));

END xilinx;


