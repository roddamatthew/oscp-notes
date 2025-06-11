---
marp: true
class: invert # Dark mode

---

# Driller: Augmenting Fuzzing Through Selective Symbolic Execution

---

- Symbolic analysis is complete but paths explode
- Fuzzing is fast but incomplete
- Static analysis can provide provable guarantees

---

# Static Analysis:

- Provides provable guarantees
- Suffers from high false positive rate of vulns
- Does not provide Proof of Compromise (PoC)

---

# Dynamic Analysis:

- Can provide PoC
- Require input test cases to "drive" execution
    - Without manual effort, their usefulness is limited
    - Quality is dependent on manual test cases

--- 

# Concolic Execution

- Hope to explore the state space more completely with program analysis and constraint solving
- Concolic tools can easily be stimied by path explosion however
    - Loop on a conditional
    - Scales geometrically in states

---

# Current SotA

- Most vulnerability testing is simply performed by fuzzing
- Its easy, and deemed "good enough"
- Nonetheless, we have bugs found everyday
- In particular, fuzzing fails to find deep bugs that relate to stateful behaviour
    - The chance of reaching this kind of bug randomly guessing is too low

---

# General vs. Structured Input

- Introduce the concept of general input:
    - Name, text, whatever
- As opposed to structured input:
    - Hash, email, phone number, IP
- Fuzzing excels at the former
- Concolic execution excels at the latter

---

# Drilling vs. Fuzzing

- Propose a model where a fuzzer and concolic engine are used in turns
- Fuzzing until we get stuck
- Concolic executing until we get stuck
- Each tool takes it in turns, hoping the other will "drill" further

---

# Related Guided Fuzzing

