"""Logger module"""
import logging
from typing import List, Union
from pathlib import Path

basehandler = logging.StreamHandler()
formatter = logging.Formatter(fmt=" %(name)s :: %(levelname)-8s :: %(message)s")
basehandler.setFormatter(formatter)


class LoggerBase:
    def __init__(self, **kwargs) -> None:
        self.init_logging()

    def init_logging(self) -> None:
        self.logger: logging.Logger = logging.getLogger(self.__class__.__name__)
        self.logger.addHandler(basehandler)
        self._logging_filehandler: Union[None, logging.FileHandler] = None

    def set_log_filehandler(self, filepath: Path) -> None:
        fh = logging.FileHandler(str(filepath), "a")
        fh.setFormatter(formatter)
        self._logging_filehandler = fh
        self.logger.addHandler(fh)

    def __del__(self) -> None:
        self.close_logger()

    def close_logger(self) -> None:
        handlers: List[logging.Handler] = self.logger.handlers
        for handler in handlers:
            handler.close()
            self.logger.removeHandler(handler)
