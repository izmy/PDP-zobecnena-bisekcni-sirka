# Úloha ZBS: zobecněná bisekční šířka

## Vstupní data:

n = přirozené číslo představující počet uzlů grafu G, n >= 10

k = přirozené číslo řádu jednotek představující průměrný stupeň uzlu grafu G, n >= k >= 3

G(V,E) = jednoduchý souvislý neorientovaný neohodnocený graf o n uzlech a průměrném stupni k

a = přirozené číslo, 5 =< a =< n/2

## Doporučení pro algoritmus generování G:

Použijte generátor grafu s volbou typu grafu „-t AD“, který vygeneruje souvislý neorientovaný neohodnocený graf o n uzlech a průměrném stupni k.

## Úkol:

Nalezněte rozdělení množiny uzlů V do dvou disjunktních podmnožin X, Y tak, že množina X obsahuje a uzlů, množina Y obsahuje n-a uzlů a počet všech hran {u,v} takových, že u je z X a v je z Y (čili velikost hranového řezu mezi X a Y), je minimální.

## Výstup algoritmu:

Výpis disjuktních podmnožin uzlů X a Y a počet hran tyto množiny spojující.

## Sekvenční algoritmus:

Řešení existuje vždy, vždy lze sestrojit bisekční šířku grafu. Sekvenční algoritmus je typu BB-DFS s hloubkou prohledávaného prostoru omezenou na |a|. Přípustný mezistav je definovaný rozdělením množiny uzlů na dvě disjunktní podmnožiny X a Y. Přípustná koncová řešení jsou všechna zkonstruovaná rozdělení množiny uzlů grafu G na podmnožiny X a Y. Cena, kterou minimalizujeme, je počet hran spojující X a Y.

Těsná dolní mez je rovna 1.

Triviální horní mez je rovna m, tj. počet hran.

## Paralelní algoritmus:

Paralelní algoritmus je typu Master-Slave.
