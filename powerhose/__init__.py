from powerhose.sender import Sender
from contextlib import contextmanager


@contextmanager
def PowerHose():
    sender = Sender()

    try:
        yield sender
    except KeyboardInterrupt:
        pass
    finally:
        import pdb; pdb.set_trace()
        sender.stop()
