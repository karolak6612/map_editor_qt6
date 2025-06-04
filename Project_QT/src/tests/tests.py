import os
import shutil

def znajdz_i_przenies_pliki_testowe(katalog_startowy=".", katalog_docelowy_nazwa="tests"):
    """
    Wyszukuje pliki zawierające "test" w nazwie w podanym katalogu startowym
    i jego podkatalogach, a następnie przenosi je do katalogu docelowego.

    Args:
        katalog_startowy (str): Ścieżka do katalogu, w którym rozpocznie się wyszukiwanie.
                                Domyślnie "." (bieżący katalog).
        katalog_docelowy_nazwa (str): Nazwa katalogu, do którego pliki zostaną przeniesione.
                                     Domyślnie "tests".
    """
    # Uzyskaj pełną, absolutną ścieżkę do katalogu startowego
    abs_katalog_startowy = os.path.abspath(katalog_startowy)
    # Utwórz pełną ścieżkę do katalogu docelowego
    abs_katalog_docelowy = os.path.join(abs_katalog_startowy, katalog_docelowy_nazwa)

    # Sprawdź, czy katalog docelowy istnieje, jeśli nie, utwórz go
    if not os.path.exists(abs_katalog_docelowy):
        try:
            os.makedirs(abs_katalog_docelowy)
            print(f"Utworzono katalog docelowy: {abs_katalog_docelowy}")
        except OSError as e:
            print(f"Błąd podczas tworzenia katalogu docelowego {abs_katalog_docelowy}: {e}")
            return # Zakończ, jeśli nie można utworzyć katalogu

    licznik_przeniesionych = 0

    # Przejdź przez strukturę katalogów
    for root, dirs, files in os.walk(abs_katalog_startowy, topdown=True):
        # Wyklucz katalog docelowy z przeszukiwania, aby uniknąć pętli
        # lub prób przenoszenia plików, które już tam są.
        # Sprawdzamy, czy 'root' (aktualnie przeszukiwany katalog)
        # jest taki sam jak katalog docelowy.
        if os.path.abspath(root) == abs_katalog_docelowy:
            # Usuwamy wszystkie podkatalogi z 'dirs', aby os.walk nie wchodził głębiej
            dirs[:] = []
            continue # Przejdź do następnego katalogu w os.walk

        for nazwa_pliku in files:
            # Sprawdź, czy "test" (ignorując wielkość liter) jest w nazwie pliku
            if "test" in nazwa_pliku.lower():
                sciezka_zrodlowa = os.path.join(root, nazwa_pliku)
                sciezka_docelowa = os.path.join(abs_katalog_docelowy, nazwa_pliku)

                # Upewnij się, że nie próbujemy przenieść pliku do samego siebie
                # (co nie powinno się zdarzyć z logiką wykluczania katalogu docelowego,
                # ale to dodatkowe zabezpieczenie)
                if os.path.abspath(sciezka_zrodlowa) == os.path.abspath(sciezka_docelowa):
                    continue

                try:
                    # Sprawdzenie, czy plik o tej samej nazwie już istnieje w docelowym
                    # Jeśli tak, można dodać logikę np. zmiany nazwy lub pominięcia
                    if os.path.exists(sciezka_docelowa):
                        print(f"Plik {nazwa_pliku} już istnieje w {abs_katalog_docelowy}. Pomijanie.")
                        # Alternatywnie, można dodać unikalny sufiks:
                        # base, ext = os.path.splitext(nazwa_pliku)
                        # i = 1
                        # while os.path.exists(sciezka_docelowa):
                        #     sciezka_docelowa = os.path.join(abs_katalog_docelowy, f"{base}_{i}{ext}")
                        #     i += 1
                        continue # Pomiń przenoszenie tego pliku

                    shutil.move(sciezka_zrodlowa, sciezka_docelowa)
                    print(f"Przeniesiono: {sciezka_zrodlowa} -> {sciezka_docelowa}")
                    licznik_przeniesionych += 1
                except Exception as e:
                    print(f"Nie można przenieść pliku {sciezka_zrodlowa}: {e}")

    if licznik_przeniesionych > 0:
        print(f"\nPrzeniesiono łącznie {licznik_przeniesionych} plików do katalogu '{katalog_docelowy_nazwa}'.")
    else:
        print(f"\nNie znaleziono żadnych plików zawierających 'test' w nazwie (poza katalogiem '{katalog_docelowy_nazwa}').")

if __name__ == "__main__":
    print("Rozpoczynanie wyszukiwania i przenoszenia plików...")
    # Uruchomienie skryptu w bieżącym katalogu
    znajdz_i_przenies_pliki_testowe()
    # Możesz też podać konkretny katalog startowy, np.:
    # znajdz_i_przenies_pliki_testowe(katalog_startowy="/sciezka/do/twojego/projektu")
    print("Zakończono.")