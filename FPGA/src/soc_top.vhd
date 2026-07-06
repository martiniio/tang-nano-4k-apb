-- soc_top.vhd
LIBRARY ieee;
USE ieee.std_logic_1164.all;

entity soc_top is
    port(
        xtal_clk, reset_n: in std_logic;
        gpio: inout std_logic_vector(1 downto 0);
        UART_TX: out std_logic;
        UART_RX: in std_logic
    );
end soc_top;

architecture structural of soc_top is
    -- Clock and reset
    signal clk_3X_out: std_logic;
    
    -- APB Master signals
    signal master_pclk     : std_logic;
    signal master_prst     : std_logic;
    signal master_penable  : std_logic;
    signal master_paddr    : std_logic_vector(7 downto 0);
    signal master_pwrite   : std_logic;
    signal master_pwdata   : std_logic_vector(31 downto 0);
    signal master_pstrb    : std_logic_vector(3 downto 0);
    signal master_pprot    : std_logic_vector(2 downto 0);
    signal master_psel1    : std_logic;
    signal master_prdata1  : std_logic_vector(31 downto 0);
    signal master_pready1  : std_logic;
    signal master_pslverr1 : std_logic;
    
    -- Internal register bank signals (connect to other IP blocks)
    signal regbank_inputs  : std_logic_vector(1023 downto 0); -- FROM other IP to regbank
    signal regbank_outputs : std_logic_vector(1023 downto 0); -- FROM regbank to other IP
    
    -- Example: signals for other IP blocks that will connect to regbank
    -- Add your actual IP block signals here as you develop them
    -- signal my_ip_status   : std_logic_vector(31 downto 0);
    -- signal my_ip_control  : std_logic_vector(31 downto 0);
    
begin

    -- Cortex-M3 with APB Master
    cortexM3_inst: entity work.Gowin_EMPU_Top
    port map (
        sys_clk => clk_3X_out,  ---81Mhz
        gpio(1 downto 0) => gpio,
        uart0_rxd => UART_RX,
        uart0_txd => UART_TX,
        reset_n => reset_n,
        
        -- APB Master Interface
        master_pclk     => master_pclk,
        master_prst     => master_prst,
        master_penable  => master_penable,
        master_paddr    => master_paddr,
        master_pwrite   => master_pwrite,
        master_pwdata   => master_pwdata,
        master_pstrb    => master_pstrb,
        master_pprot    => master_pprot,
        master_psel1    => master_psel1,
        master_prdata1  => master_prdata1,
        master_pready1  => master_pready1,
        master_pslverr1 => master_pslverr1
    );
    
    -- PLL for clock generation
    pllvr_inst: entity work.Gowin_PLLVR
    port map (
        clkout => clk_3X_out, 
        clkin => xtal_clk --27M
    );
    
    -- APB Slave Register Bank
    apb_slave_inst: entity work.apb_slave_regbank
    port map (
        -- APB Interface (from Cortex-M3 master)
        PCLK        => master_pclk,
        PRESETn     => master_prst,
        PADDR       => master_paddr,
        PSEL        => master_psel1,
        PENABLE     => master_penable,
        PWRITE      => master_pwrite,
        PWDATA      => master_pwdata,
        PSTRB       => master_pstrb,
        PRDATA      => master_prdata1,
        PREADY      => master_pready1,
        PSLVERR     => master_pslverr1,
        
        -- Register Interface (internal connections to other IP)
        reg_inputs      => regbank_inputs,   -- Data FROM other IP blocks
        reg_outputs     => regbank_outputs   -- Data TO other IP blocks
    );
    
    -- =====================================================
    -- Connect your other IP blocks here
    -- =====================================================
    -- Example: Connect a status register from some IP block
    -- regbank_inputs(31 downto 0) <= my_ip_status;
    -- my_ip_control <= regbank_outputs(31 downto 0);
    
    -- For now, tie unused inputs to zero (remove as you add IP blocks)
    regbank_inputs <= (others => '0');
    
    -- You can add more IP blocks like:
    -- my_custom_ip_inst: entity work.my_custom_ip
    --   port map (
    --     clk => master_pclk,
    --     rst_n => master_prst,
    --     status_out => regbank_inputs(63 downto 32),
    --     control_in => regbank_outputs(63 downto 32),
    --     ...
    --   );
    
end structural;