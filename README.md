![CodeBase](https://progress-bar.dev/15/?title=Codebase)
![MIT](https://img.shields.io/badge/License-MIT-green)
![Black](https://img.shields.io/badge/Style-Black-black)
# LostTraderBot
The lost bot looking for a new future!


# TODO (Event Base backend)
- Add dependencies and export them as requirements `pip freeze` sort of thing
- Add unit tests

# TODO (Strategies)
- Add multiple indicators: Hurst exponent, Ichimoku cloud, SuperTrend, etc.
- Add order book dynamic, i.e. level 1, 2, 3 volume as a way to determine market direction.
- Dynamic book depth to limit exposure to position limits.
- Test preferences to higher frequency to introduce liquidity in the market vs. long position over time.
- Crossing the book dynamics
- Manage inventory in general 

## Instalation 
To use this library, install it as ant usual PIPy package: 

```shell
pip3 install -e .  
```

To install it with requirements (using `requirements.txt`) use:

```shell
pip3 install --requirement requirements.txt -e .
```

# Python style
- [Black](https://github.com/psf/black)
- [PEP8](https://peps.python.org/pep-0008/)
- [Google Python Style](https://google.github.io/styleguide/pyguide.html)

# References for implementation
- Successful Algorithmic Trading, Michael L. Halls-Moore


# Contribution
- Currently working alone on the project, if someone wants to contibute, feel free to raise issues!
- Long term is to make it a long standing - fully operative project. I'm willing to risk a few hundreds (EUR)! 

Under MIT License
