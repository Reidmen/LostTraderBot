from setuptools import setup, find_packages

# load README file and use it as long_description for PyPI
with open("README.md", "r") as file_:
    readme = file_.read()

# package configuration
setup(
    name="losttraderbot",
    version="0.1.0",
    author="Reidmen Arostica",
    author_email="r.rethmawn@gmail.com",
    description="Event-driven Backend Tester",
    long_description=readme,
    long_description_content_type="text/markdown",
    packages=["losttraderbot"],
    include_package_data=True,
    python_requires=">3.8.*",
    license="MIT License",
    zip_safe=False,
    classifiers=[
        "Development Statys :: 3 - Alpha",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Intended Audience :: Developers",
        "Programming Language :: Python :: 3.8",
    ],
    install_requires=[
        "matplotlib==3.7.1",
        "numpy==1.24.2",
        "pandas==1.5.3",
        "scipy==1.10.1",
        "statsmodels==0.13.5",
        "yfinance==0.2.12",
    ],
    keywords="losttraderbot event-driven backtester",
)
