from powerhose.sender import Sender
from contextlib import contextmanager


@contextmanager
def PowerHose():
    sender = Sender()
    try:
        yield sender
    finally:
        sender.stop()
