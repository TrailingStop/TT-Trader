# TT-Trader
This is not a financial advice!


This is the home of TT-Trader. An command line trading framework for crypto trading. I'll release the SDK (C++ only) for TT-Trader soon.

If you use the TT-Trader affiliate link to open your account thew development of the  algo will be free of charge. Just get in touch with us by mail: 	
support@tt-trader.de


## features
- trade on all supported exchange from a single algo at the same time (arbitrage)
- use market data from all supported exchanges
- uses JSON files to configure the algos (please see this sample file: https://github.com/TrailingStop/TT-Trader/blob/main/2BarHighLow.cfg)
- separate configurable stop loss / take profit modules
- predefined indicators
- option to use auto adjusting limit orders (maker) instead of market orders (take)
- monitor the entry/exit of the algo on the exchanges web frontend


## supported instruments
- perpetual futures


## supported exchanges
- BitMart
  - referral link: https://www.bitmart.com/invite/c6nXwm/en-US
  - I can also help to get a special fee arrangement
- ByBit (in development)
- KuCoin (in development)
- HTX (in development)
- let me know if you miss an exchange


## typical screenshot running an algo with 2 orders open
- mixed timeframe (30 and 15 minutes)
- BTCUSDT only
- exit modules TakeProfit and StopLossBarBased activated
- trading range check activated - switches trading off if volatility drops below a certain value

![image](https://github.com/user-attachments/assets/8a63876d-983a-4c16-8f88-725c64b73b6d)


closed position (long) including some order/position information

![image](https://github.com/user-attachments/assets/c1b7738d-8b52-4653-a9c8-98d8ebd1dfc0)

