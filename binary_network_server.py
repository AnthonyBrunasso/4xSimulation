import asyncio
import socket
import struct

binary_game_state = []
server_in_use = asyncio.Event()
connected_clients = 0

async def binary_reader(reader):
  global binary_game_state
  while True:
    try:
      header = await reader.readexactly(4)
      print('Header completed', header)
      header_size, = struct.unpack('I', header)
      print('Computed body size', header_size)
      body = await reader.readexactly(header_size)
      print('Message read')
    except asyncio.streams.IncompleteReadError as e:
      print('Stream read failed')
      return
    # Game message read
    message = bytearray()
    message += header
    message += body
    binary_game_state.append(message)
  
async def binary_writer(writer):
  message_index = 0
  while True:
    print('writer tick')
    await asyncio.sleep(1)
    if len(binary_game_state) <= message_index:
      continue
    print('message queued')
    message = binary_game_state[message_index]
    message_index += 1
    writer.write(message)
    await writer.drain()
    print('message sent', len(message), 'bytes')

async def handle_binary_game_client(reader, writer):
  global server_in_use, connected_clients
  server_in_use.set()
  connected_clients += 1
  print('New client connection')
  tasks = [
     asyncio.ensure_future(binary_reader(reader)),
     asyncio.ensure_future(binary_writer(writer)),
  ]
  done, pending = await asyncio.wait(tasks, return_when=asyncio.FIRST_COMPLETED)
  for p in pending:
    p.cancel()
  connected_clients -= 1
  print('Lost client connection')
  
async def auto_exit(server):
  await server_in_use.wait()
  print('a game began')

  # Wait 12 cycles with no active players before exiting
  def reset_cycle():
    return 12
  cycle = reset_cycle()
  while cycle > 0:
    await asyncio.sleep(5)
    # While clients are connected, wait
    if connected_clients > 0:
      print('server has active clients')
      cycle = reset_cycle()
      continue
    print('server has NO active clients')
    cycle -=  1
    
async def acceptor(port):
  server = await asyncio.start_server(handle_binary_game_client, host='0.0.0.0', port=port, backlog=8)
  print('Server is listening on port', port)
  tasks = [
          asyncio.ensure_future(server.wait_closed()),
          asyncio.ensure_future(auto_exit(server))
  ]

  done, pending = await asyncio.wait(tasks, return_when=asyncio.FIRST_COMPLETED)
  for p in pending:
    p.cancel()
  print('Exiting')

if __name__ == "__main__":
  loop = asyncio.get_event_loop()
  loop.run_until_complete(acceptor(4000))
