Title: Making Sense Out of the Matlab FFT Results
Date: 2011-10-11
Modified: 2017-01-01
Category: Technology
Tags: Math

Here's a note for people who know nothing about digital signal processing (like me) to make sense out of the matlab FFT results.

For a input A of N real numbers, B = fft(A) contains N complex numbers.  B has the following property:

- B[0] (the first number, DC) is pure real.
- If N is even, then B[N/2] (the “Nyquist” frequency) is also pure real.
- Other than that, B[i] = B[N-i]*.

If we take the abs of the output, then the first ceil(N/2) elements contain all the information.  Now, what's important about this abs of fft output is that it represents the power of components of various frequency.  That is:

abs(B[n]) is the magnitude of the component, a sine wave, that is sampled n cycles by A.

FFT needs to sample at least two points from each cycle, so the components with the highest frequency that appears in the FFT result is sampled floor(N/2) cycles, and that corresponds to B[floor(N/2)].

So far we have not talked anything about sample rate.  Now, assume that the sample rate of A is r, then the time length of A is N/r seconds.  For B[n], A covers n cycles, so each cycle is N/(nr) seconds.  That is, the corresponding frequency of B[n] is nr/N.

So here's the functionality of sample rate r and number of samples N:

1.  Sample rate r controls the highest frequency, that is r/2, that is covered by FFT.
2. N controls the granularity of the quantization of the frequency range [0, r/2].
