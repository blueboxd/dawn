#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void a() {
  int a_1 = 0;
  switch(a_1) {
    case 0:
    case 2:
    case 4: {
      uint b = 3u;
      switch(b) {
        case 0u: {
          break;
        }
        case 1u:
        case 2u:
        case 3u:
        default: {
          uint c = 123u;
          switch(c) {
            case 0u: {
              break;
            }
            default: {
              return;
              break;
            }
          }
          return;
          break;
        }
      }
      break;
    }
    case 1:
    default: {
      return;
      break;
    }
  }
}

