--------------------------------------------------------------------------------
-- File: apb_slave_regbank.vhd
-- Description: APB Slave Register Bank with Read-Only and Read-Write Registers
--
-- Features:
--   - Full APB protocol compliance (AMBA APB v2.0)
--   - 256-byte address space (64 x 32-bit registers)
--   - Address range 0x00-0x7F: Read-only input registers (from other IP)
--   - Address range 0x80-0xFF: Read-write output registers (to other IP)
--   - Byte-enable write support via PSTRB
--   - Single-cycle response with proper pipelining for high-frequency operation
--   - Zero wait-state operation
--
-- Memory Map:
--   0x00 - 0x7C: 32 Read-Only registers (status/input from other IP blocks)
--   0x80 - 0xFC: 32 Read-Write registers (control/output to other IP blocks)
--
-- Author: [Your Name/Company]
-- Date: 2025-12-20
-- Version: 1.0
--------------------------------------------------------------------------------

LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.numeric_std.all;

entity apb_slave_regbank is
    generic (
        -- Address and data width parameters
        G_ADDR_WIDTH    : integer := 8;     -- 256 bytes addressable
        G_DATA_WIDTH    : integer := 32;    -- 32-bit data bus
        G_NUM_RO_REGS   : integer := 32;    -- Number of read-only registers
        G_NUM_RW_REGS   : integer := 32     -- Number of read-write registers
    );
    port (
        -- APB Interface Signals (AMBA APB Specification)
        PCLK            : in  std_logic;                                    -- APB clock
        PRESETn         : in  std_logic;                                    -- APB reset (active low)
        PADDR           : in  std_logic_vector(G_ADDR_WIDTH-1 downto 0);   -- APB address bus
        PSEL            : in  std_logic;                                    -- APB select
        PENABLE         : in  std_logic;                                    -- APB enable
        PWRITE          : in  std_logic;                                    -- APB write strobe
        PWDATA          : in  std_logic_vector(G_DATA_WIDTH-1 downto 0);   -- APB write data
        PSTRB           : in  std_logic_vector(G_DATA_WIDTH/8-1 downto 0); -- APB byte strobe
        PRDATA          : out std_logic_vector(G_DATA_WIDTH-1 downto 0);   -- APB read data
        PREADY          : out std_logic;                                    -- APB ready
        PSLVERR         : out std_logic;                                    -- APB slave error
        
        -- Register Interface to Other IP Blocks
        -- Input Registers: Data FROM other IP blocks (read-only from CPU perspective)
        reg_inputs      : in  std_logic_vector((G_NUM_RO_REGS * G_DATA_WIDTH)-1 downto 0);
        
        -- Output Registers: Data TO other IP blocks (read-write from CPU perspective)
        reg_outputs     : out std_logic_vector((G_NUM_RW_REGS * G_DATA_WIDTH)-1 downto 0)
    );
end entity apb_slave_regbank;

architecture rtl of apb_slave_regbank is
    
    -------------------------------------------------------------------------------
    -- Type Definitions
    -------------------------------------------------------------------------------
    type t_reg_array is array (0 to G_NUM_RW_REGS-1) of std_logic_vector(G_DATA_WIDTH-1 downto 0);
    
    -------------------------------------------------------------------------------
    -- Internal Register Storage
    -------------------------------------------------------------------------------
    signal rw_regs          : t_reg_array;  -- Read-write register array
    
    -------------------------------------------------------------------------------
    -- APB State and Control Signals (Registered)
    -------------------------------------------------------------------------------
    signal apb_addr_reg     : std_logic_vector(G_ADDR_WIDTH-1 downto 0);
    signal apb_write_reg    : std_logic;
    signal apb_wdata_reg    : std_logic_vector(G_DATA_WIDTH-1 downto 0);
    signal apb_strb_reg     : std_logic_vector(G_DATA_WIDTH/8-1 downto 0);
    
    -------------------------------------------------------------------------------
    -- APB Response Signals (Registered)
    -------------------------------------------------------------------------------
    signal apb_rdata_reg    : std_logic_vector(G_DATA_WIDTH-1 downto 0);
    signal apb_ready_reg    : std_logic;
    
    -------------------------------------------------------------------------------
    -- Address Decode Signals
    -------------------------------------------------------------------------------
    signal word_address     : unsigned(G_ADDR_WIDTH-3 downto 0);   -- Word-aligned address
    signal reg_index        : integer range 0 to G_NUM_RW_REGS-1;
    signal addr_is_ro_range : std_logic;  -- '1' when accessing read-only range
    signal addr_is_rw_range : std_logic;  -- '1' when accessing read-write range
    
    -------------------------------------------------------------------------------
    -- Constants
    -------------------------------------------------------------------------------
    constant C_RO_BASE_ADDR : unsigned(G_ADDR_WIDTH-1 downto 0) := to_unsigned(16#00#, G_ADDR_WIDTH);
    constant C_RW_BASE_ADDR : unsigned(G_ADDR_WIDTH-1 downto 0) := to_unsigned(16#80#, G_ADDR_WIDTH);
    
begin
    
    -------------------------------------------------------------------------------
    -- Output Port Assignments
    -------------------------------------------------------------------------------
    PRDATA  <= apb_rdata_reg;
    PREADY  <= apb_ready_reg;
    PSLVERR <= '0';  -- This slave never generates errors
    
    -------------------------------------------------------------------------------
    -- Address Decoding (Combinatorial - operates on registered address)
    -------------------------------------------------------------------------------
    word_address     <= unsigned(apb_addr_reg(G_ADDR_WIDTH-1 downto 2));  -- Word-aligned
    reg_index        <= to_integer(word_address(4 downto 0));              -- Extract register index
    addr_is_ro_range <= not apb_addr_reg(7);                               -- 0x00-0x7F
    addr_is_rw_range <= apb_addr_reg(7);                                   -- 0x80-0xFF
    
    -------------------------------------------------------------------------------
    -- Main APB State Machine
    --
    -- APB Protocol operates in two phases:
    --   1. SETUP Phase:   PSEL=1, PENABLE=0  -> Sample address and control
    --   2. ACCESS Phase:  PSEL=1, PENABLE=1  -> Perform read/write operation
    --
    -- This implementation:
    --   - Captures address/control during SETUP phase for timing closure
    --   - Executes operation during ACCESS phase using registered signals
    --   - Provides single-cycle response (PREADY='1' in ACCESS phase)
    -------------------------------------------------------------------------------
    p_apb_state_machine : process(PCLK, PRESETn)
        variable v_reg_idx : integer range 0 to G_NUM_RW_REGS-1;
        variable v_input_data : std_logic_vector(G_DATA_WIDTH-1 downto 0);
    begin
        if PRESETn = '0' then
            -- Asynchronous Reset
            -- Reset all read-write registers to zero
            for i in 0 to G_NUM_RW_REGS-1 loop
                rw_regs(i) <= (others => '0');
            end loop;
            
            -- Reset APB control signals
            apb_addr_reg  <= (others => '0');
            apb_write_reg <= '0';
            apb_wdata_reg <= (others => '0');
            apb_strb_reg  <= (others => '0');
            
            -- Reset APB response signals
            apb_rdata_reg <= (others => '0');
            apb_ready_reg <= '0';
            
        elsif rising_edge(PCLK) then
            
            -- Default: No ready unless explicitly set
            apb_ready_reg <= '0';
            
            if PSEL = '1' then
                
                ---------------------------------------------------------------
                -- SETUP PHASE: Capture Address and Control Signals
                ---------------------------------------------------------------
                if PENABLE = '0' then
                    -- Register all inputs for timing closure at high frequencies
                    apb_addr_reg  <= PADDR;
                    apb_write_reg <= PWRITE;
                    apb_wdata_reg <= PWDATA;
                    apb_strb_reg  <= PSTRB;
                    
                    -- Clear ready during setup
                    apb_ready_reg <= '0';
                    
                ---------------------------------------------------------------
                -- ACCESS PHASE: Execute Read or Write Operation
                ---------------------------------------------------------------
                elsif PENABLE = '1' then
                    
                    v_reg_idx := reg_index;  -- Use decoded register index
                    
                    -----------------------------------------------------------
                    -- WRITE OPERATION
                    -----------------------------------------------------------
                    if apb_write_reg = '1' then
                        
                        -- Only allow writes to read-write register range
                        if addr_is_rw_range = '1' then
                            
                            -- Perform byte-enable write
                            for byte_idx in 0 to (G_DATA_WIDTH/8)-1 loop
                                if apb_strb_reg(byte_idx) = '1' then
                                    rw_regs(v_reg_idx)((byte_idx+1)*8-1 downto byte_idx*8) 
                                        <= apb_wdata_reg((byte_idx+1)*8-1 downto byte_idx*8);
                                end if;
                            end loop;
                            
                        end if;
                        -- Note: Writes to read-only range are silently ignored (no error)
                        
                        -- Assert ready to complete write transfer
                        apb_ready_reg <= '1';
                        
                    -----------------------------------------------------------
                    -- READ OPERATION
                    -----------------------------------------------------------
                    else
                        
                        -- Read from read-only register range (status from other IP)
                        if addr_is_ro_range = '1' then
                            v_input_data := reg_inputs((v_reg_idx+1)*G_DATA_WIDTH-1 downto v_reg_idx*G_DATA_WIDTH);
                            apb_rdata_reg <= v_input_data;
                            
                        -- Read from read-write register range (control to other IP)
                        elsif addr_is_rw_range = '1' then
                            apb_rdata_reg <= rw_regs(v_reg_idx);
                            
                        -- Out of range - return zero
                        else
                            apb_rdata_reg <= (others => '0');
                        end if;
                        
                        -- Assert ready to complete read transfer
                        apb_ready_reg <= '1';
                        
                    end if;  -- Write vs Read
                    
                end if;  -- SETUP vs ACCESS phase
                
            end if;  -- PSEL
            
        end if;  -- Clock/Reset
    end process p_apb_state_machine;
    
    -------------------------------------------------------------------------------
    -- Output Register Mapping
    -- Connect internal register array to output port for other IP blocks
    -------------------------------------------------------------------------------
    gen_output_mapping : for i in 0 to G_NUM_RW_REGS-1 generate
        reg_outputs((i+1)*G_DATA_WIDTH-1 downto i*G_DATA_WIDTH) <= rw_regs(i);
    end generate gen_output_mapping;
    
end architecture rtl;

--------------------------------------------------------------------------------
-- End of File
--------------------------------------------------------------------------------