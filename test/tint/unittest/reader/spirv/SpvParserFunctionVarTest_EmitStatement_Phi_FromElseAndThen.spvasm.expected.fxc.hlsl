SKIP: FAILED

static uint x_1 = 0u;
static bool x_7 = false;
static bool x_8 = false;

void main_1() {
  const bool x_101 = x_7;
  const bool x_102 = x_8;
  while (true) {
    uint x_2_phi = 0u;
    if (x_101) {
      break;
    }
    if (x_102) {
      x_2_phi = 0u;
      {
        x_1 = x_2_phi;
      }
      continue;
    } else {
      x_2_phi = 1u;
      {
        x_1 = x_2_phi;
      }
      continue;
    }
    {
      x_1 = x_2_phi;
    }
  }
  return;
}

void main() {
  main_1();
  return;
}
FXC validation failure:
C:\src\dawn\test\tint\Shader@0x0000025B50880850(8,17-20): error X3696: infinite loop detected - loop never exits

