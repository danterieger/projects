"""Re-check of the wind turbine tower model (3 lumped masses + optional absorber).

Steady-state response to a 500 kN harmonic force on each tower mass, solved directly
from (K - w^2 M) X = F, so natural frequencies and mode shapes can't be mis-paired.
A small loss factor (eta) is added to every spring only so resonance peaks stay finite
on the plot; the original model was undamped.
"""
import numpy as np
import matplotlib.pyplot as plt

m1 = m2 = 207_000.0          # lumped tower masses, kg
m3 = 129_144.0               # nacelle and generator, kg
k1 = k3 = 78.3e6             # quarter-length tower segments, N/m
k2 = 9.79e6                  # half-length segment, N/m
m4 = 50_000.0                # absorber mass, kg
F0 = 500e3                   # excitation amplitude on each tower mass, N
eta = 0.01                   # loss factor for plotting only

def tower(k4=None):
    K = np.array([[k1 + k2, -k2, 0], [-k2, k2 + k3, -k3], [0, -k3, k3]], float)
    M = np.diag([m1, m2, m3])
    F = np.full(3, F0)
    if k4 is None:
        return K, M, F
    K4 = np.zeros((4, 4)); K4[:3, :3] = K
    K4[2, 2] += k4; K4[2, 3] -= k4; K4[3, 2] -= k4; K4[3, 3] += k4   # absorber on the nacelle
    return K4, np.diag([m1, m2, m3, m4]), np.append(F, 0.0)

def top_amplitude(f_hz, k4=None):
    K, M, F = tower(k4)
    w = 2 * np.pi * f_hz
    return abs(np.linalg.solve(K * (1 + 1j * eta) - w**2 * M, F)[2])

K, M, _ = tower()
fn = np.sqrt(np.sort(np.linalg.eigvals(np.linalg.solve(M, K)).real)) / (2 * np.pi)
print("Natural frequencies (Hz):", np.round(fn, 2))

k_original = 100e3
k_tuned = m4 * (2 * np.pi * fn[2]) ** 2
print(f"Original absorber tuned to {np.sqrt(k_original / m4) / (2 * np.pi):.2f} Hz")
print(f"Tuned absorber stiffness: {k_tuned / 1e6:.1f} MN/m")

f = np.linspace(3.5, 6.5, 1200)
plt.figure(figsize=(9, 3.8))
plt.semilogy(f, [top_amplitude(x) for x in f], color="#10203a", lw=2, label="Tower alone")
plt.semilogy(f, [top_amplitude(x, k_original) for x in f], color="#8a97ab", lw=1.5, ls="--", label="Original absorber (100 kN/m)")
plt.semilogy(f, [top_amplitude(x, k_tuned) for x in f], color="#1e5aa8", lw=2.2, label=f"Tuned absorber ({k_tuned / 1e6:.0f} MN/m)")
plt.axvspan(4, 6, color="#f5c518", alpha=0.18, label="Excitation band, 4 to 6 Hz")
plt.xlabel("Excitation frequency (Hz)"); plt.ylabel("Nacelle amplitude (m)")
plt.grid(which="both", color="#dde5f0", lw=0.6); plt.legend(frameon=False, fontsize=9)
plt.tight_layout(); plt.savefig("images/recheck-response.png", dpi=150, facecolor="white")
