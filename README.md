# logue collection

This project is a collection of custom audio units for Korg NTS-1.

## Units

### Oscillators

#### fm4o

4 operators FM oscillator in this configuration.

```
 _____
/     \    Feedback
|     |
\-- [ O1 ] Shape             [ O3 ] O3o
      |                        |
      *    Shift + LFO         *    O3 Mod. Depth + LFO
      |                        |
    [ O2 ] O2 Octave         [ O4 ] O4 Octave
      |                        |
      \------------------------/    Balance
```

| Parameter     | Function                                            |
|---------------|-----------------------------------------------------|
| Shape         | Frequency ratio of O1 (continuous from 0.25 to 16)  |
| Shift         | Modulation depth of O1 over O2                      |
| Feedback      | O1 feefback                                         |
| O2 Octave     | Frequency ratio of O2 (only octaves from -2 to + 2) |
| O3 Octave     | Frequency ratio of O3 (only octaves from -2 to + 2) |
| O3 Mod. Depth | Modulation depth of O3 over O4                      |
| O4 Octave     | Frequency ratio of O4 (only octaves from -2 to + 2) |
| Balance       | Audio balance between O2 and O4                     |

#### mass

Multi saw oscillator with sub oscillator.

| Parameter    | Function              |
|--------------|-----------------------|
| Shape        | Voice detuning        |
| Shift        | Sub oscillator mix    |
| Voices (1-7) | Number of voices      |
| Beating      | Voice linear detuning |

### Modulation Effects

#### sola

Dual mono Solina chorus recreation. Set both parameters at noon for the classical sound.

| Parameter | Function |
|-----------|----------|
| Time      | Speed    |
| Depth     | Depth    |

### Delay Effects

#### rock

Dual mono 4 pole phaser with phase difference between channels implemented as a delay FX to use it with other modulation FX, specially sola.

| Parameter | Function                    |
|-----------|-----------------------------|
| Time      | Speed                       |
| Depth     | Feedback                    |
| Mix       | Phase difference (0 - 180º) |

## Installation

Requirements:

- Docker
- GNU make

## Compilation

Just run `make image; make`. All the `ntkdigunit` files will be installed in the `platform/bin` directory.

Notice that `make image` builds the logue-sdk Docker image and it is only needed to be run once.

## logue CLI

Just run `make cli` and a softlink `logue-cli` pointing to the binary tool will be created at the project root directory.
