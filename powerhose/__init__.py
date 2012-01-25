from powerhose.sender import Sender
from contextlib import contextmanager


@contextmanager
def PowerHose(identifier, prefix=None):
    sender = Sender(identifier, prefix)

    try:
        yield sender
    except KeyboardInterrupt:
        pass
    finally:
        sender.stop()
