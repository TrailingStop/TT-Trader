# TT-Trader
This is not a financial advice!


This is the home of TT-Trader. An command line trading framework for crypto trading. I'll release the SDK (C++ only) for TT-Trader soon.

If you use the TT-Trader affiliate link to open your account the development of an algo following your trading rules will be free of charge. Just get in touch with us by mail: 	
support@tt-trader.de and enjoy the full service we offer (adding new indicators, exchanges, instruments and algos as well as stop loss and take profit exit modules)


## how it works
You open your account with the referral link you see for each supported exchange und send us your account id. We will then compile a license file for you which is needed to use the exchange with TT-Trader. Together we work out how your algo should work and we will develop it for you. As soon as it is ready to go (expect about 5 days of work) you will receive your personal algo and you can start TT-Trader. In most cases we expect to make some adjustments to the algo which will be the next step. The development and the adjustments are free of charge for you. The cost are covered if you open your account with our referral link. If you still have questions please let us know and use the mail support@tt-trader.de


## free for you
If you open your account with our referral link you get all our services for free, the cost are covered by the agreement with the exchange. On top of this we have an agreement with some exchanges that will get you a discount on the regular fees.


## save fees with TT-Trader - use maker orders, not taker
Beside the option to get a better base fee rate with TT-Trader, you can also make sure that your order will execute as maker. This makes sure that you be on the cheaper side of the execution.


## features
- trade on all supported exchange from a single algo at the same time (arbitrage)
- use market data from all supported exchanges
- uses JSON files to configure the algos (please see this sample file: https://github.com/TrailingStop/TT-Trader/blob/main/2BarHighLow.cfg)
- separate configurable stop loss / take profit modules
- predefined indicators
- option to use auto adjusting limit orders (maker) instead of market orders (take)
- monitor the entry/exit of the algo on the exchanges web frontend
- simulated stop market & stop limit orders if not supported by the exchange
- simulated modify (cancel/replace) if not supported by the exchange


## supported instruments
- perpetual futures


## supported exchanges
- BitMart
  - referral link: https://www.bitmart.com/invite/c6nXwm/en-US
  - I can also help to get a special fee arrangement
- ByBit (in development)
- KuCoin (in development)
- HTX (in development)
- let us know if you miss an exchange


## typical screenshot running an algo with 2 orders open
- mixed timeframe (30 and 15 minutes)
- BTCUSDT only
- exit modules TakeProfit and StopLossBarBased activated
- trading range check activated - switches trading off if volatility drops below a certain value

![image](https://github.com/user-attachments/assets/8a63876d-983a-4c16-8f88-725c64b73b6d)


closed position (long) including some order/position information

![image](https://github.com/user-attachments/assets/c1b7738d-8b52-4653-a9c8-98d8ebd1dfc0)

