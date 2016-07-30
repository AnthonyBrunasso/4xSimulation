import asyncio
import socket
import struct

binary_game_state = []

@asyncio.coroutine
def binary_reader(reader):
  global binary_game_state
  while True:
    try:
      header = yield from reader.readexactly(4)
      print('Header completed', header)
      header_size, = struct.unpack('I', header)
      print('Computed body size', header_size)
      body = yield from reader.readexactly(header_size)
      print('Message read')
    except asyncio.streams.IncompleteReadError as e:
      print('Client connection lost')
      return
    # Game message read
    message = bytearray()
    message += header
    message += body
    binary_game_state.append(message)
  
@asyncio.coroutine
def binary_writer(writer):
  message_index = 0
  while True:
    print('writer tick')
    yield from asyncio.sleep(1)
    if len(binary_game_state) <= message_index:
      continue
    print('message queued')
    message = binary_game_state[message_index]
    message_index += 1
    writer.write(message)
    yield from writer.drain()
    print('message sent', len(message), 'bytes')
    
@asyncio.coroutine
def binary_game_client(reader, writer):
  tasks = [
     asyncio.async(binary_reader(reader)),
     asyncio.async(binary_writer(writer)),
  ]
  done, pending = yield from asyncio.wait(tasks, return_when=asyncio.FIRST_COMPLETED)
  print('Connection read/write finalization')
  for p in pending:
    p.cancel()
  print('Connection read/write exiting')
  
@asyncio.coroutine
def acceptor():
  server = yield from asyncio.start_server(binary_game_client, host='0.0.0.0', port=4000, backlog=8)
  yield from server.wait_closed()
  print('Exiting')

if __name__ == "__main__":
  loop = asyncio.get_event_loop()
  loop.run_until_complete(acceptor())

  # Run the event loop
  loop.run_forever()
