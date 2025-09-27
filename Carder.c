/*
    Carder is a word book application that you can
    save new cards and view them as you want.
    It saves card informations to the file in the
    executable directory.

    Usage:
        To iterate a deck:
            carder Existing.deck

        To start building a deck:
            carder new New.deck
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define STRING_BUFFER_SIZE 4096
#define INPUT_STOP_STRING "!done"

typedef struct card
{
    size_t id;

    char *front;
    size_t frontLength;

    char *back;
    size_t backLength;
} card;

typedef struct deck
{
    char *name;
    card *cards;
    size_t size;
} deck;

FILE *CURRENT_DECK_FILE = NULL;

deck *deckRead(deck *deckBuffer, char *fileName)
{
    deckBuffer->name = fileName;
    fopen_s(&CURRENT_DECK_FILE, deckBuffer->name, "r");

    if (CURRENT_DECK_FILE == NULL)
    {
        printf("File '%s' not found.", deckBuffer->name);
        return NULL;
    }

    char line[STRING_BUFFER_SIZE] = {0};
    card *currentCard = NULL;
    size_t cardIndex = 0;

    while (fgets(line, sizeof(line), CURRENT_DECK_FILE) != NULL)
    {
        if (!strncmp("card", line, 4))
        {
            currentCard = deckBuffer->cards + cardIndex++;
            currentCard->id = atoi(line + 5);
        }
        else if (!strncmp("flen", line, 4))
        {
            currentCard->frontLength = atoi(line + 5);
        }
        else if (!strncmp("fstr", line, 4))
        {
            currentCard->front = (char *)malloc(currentCard->frontLength + 1);
            memcpy(currentCard->front, line + 5, currentCard->frontLength);
            currentCard->front[currentCard->frontLength] = '\0';
        }
        else if (!strncmp("blen", line, 4))
        {
            currentCard->backLength = atoi(line + 5);
        }
        else if (!strncmp("bstr", line, 4))
        {
            currentCard->back = (char *)malloc(currentCard->backLength + 1);
            memcpy(currentCard->back, line + 5, currentCard->backLength);
            currentCard->back[currentCard->backLength] = '\0';
        }
        else if (!strncmp("deck", line, 4))
        {
            deckBuffer->size = atoi(line + 5);
            deckBuffer->cards = (card *)malloc(deckBuffer->size * sizeof(card));
        }
        else
        {
            // printf("Unrecognized token : '%s'", line);
            // fclose(CURRENT_DECK_FILE);
            // return NULL;

            continue;
        }
    }

    fclose(CURRENT_DECK_FILE);

    return deckBuffer;
}

int deckWrite(deck deckToWrite)
{
    remove(deckToWrite.name);
    fopen_s(&CURRENT_DECK_FILE, deckToWrite.name, "w");

    if (CURRENT_DECK_FILE == NULL)
    {
        printf("File open failed for '%s'.", deckToWrite.name);
        return 1;
    }

    fprintf(CURRENT_DECK_FILE, "deck %zu\n\n", deckToWrite.size);

    for (size_t i = 0; i < deckToWrite.size; i++)
    {
        card *cardToWrite = deckToWrite.cards + i;
        fprintf(CURRENT_DECK_FILE, "card %zu\nflen %zu\nfstr %s\nblen %zu\nbstr %s\n\n",
                cardToWrite->id,
                cardToWrite->frontLength, cardToWrite->front,
                cardToWrite->backLength, cardToWrite->back);
    }

    fclose(CURRENT_DECK_FILE);

    return 0;
}

deck *deckBuild(deck *deckBuffer)
{
    printf("\nEntered build mode. Type '!done' to finish.\n\n");

    char input[STRING_BUFFER_SIZE] = {0};
    card *currentCard = NULL;
    int onFront = 1;
    size_t inputStopLength = strlen(INPUT_STOP_STRING);

    printf("Enter front side of the card:\n");

    while (fgets(input, sizeof(input), stdin))
    {
        size_t inputLen = strlen(input);

        if (inputLen == 1)
        {
            continue;
        }

        if (!strncmp(input, INPUT_STOP_STRING, inputStopLength))
        {
            if (onFront)
            {
                printf("Deck building finished.\n");
                break;
            }
            else
            {
                printf("Please enter the back of the card.\n");
                continue;
            }
        }

        if (onFront)
        {
            deckBuffer->size++;
            deckBuffer->cards = realloc(deckBuffer->cards, deckBuffer->size * sizeof(card));

            if (deckBuffer->cards == NULL)
            {
                printf("Memory allocation failed for new card.");
                return NULL;
            }

            currentCard = deckBuffer->cards + deckBuffer->size - 1;
            currentCard->id = deckBuffer->size;
            currentCard->frontLength = inputLen - 1;
            currentCard->front = (char *)malloc(currentCard->frontLength + 1);
            memcpy(currentCard->front, input, currentCard->frontLength);
            currentCard->front[currentCard->frontLength] = '\0';
            printf("Enter back side of the card:\n");
        }
        else
        {
            currentCard->backLength = inputLen - 1;
            currentCard->back = (char *)malloc(currentCard->backLength + 1);
            memcpy(currentCard->back, input, currentCard->backLength);
            currentCard->back[currentCard->backLength] = '\0';
            printf("\nEnter front side of the card:\n");
        }

        onFront = !onFront;
    }

    return deckBuffer;
}

void deckDestroy(deck *deckToDestroy)
{
    for (size_t i = 0; i < deckToDestroy->size; i++)
    {
        card *cardToDestroy = deckToDestroy->cards + i;
        cardToDestroy->id = 0;
        cardToDestroy->backLength = 0;
        cardToDestroy->frontLength = 0;
        free(cardToDestroy->back);
        free(cardToDestroy->front);
    }

    free(deckToDestroy->cards);
    deckToDestroy->cards = NULL;
    deckToDestroy->size = 0;
    deckToDestroy->name = NULL;
}

int main(int argc, char **argv)
{
    if (!(argc == 3 && !strcmp("new", argv[1])) && argc != 2)
    {
        printf("\nPlease enter which deck you want to use. Or 'new + \"DeckName.deck\" if you want to create new.\n");
        return 1;
    }

    if (argc == 2) // carder Existing.deck
    {
        deck existingDeck = {0};
        if (deckRead(&existingDeck, argv[1]) == NULL)
        {
            return 1;
        }

        for (size_t i = 0; i < existingDeck.size; i++)
        {
            card currentCard = existingDeck.cards[i];
            printf("card id : %zu, front : '%s', back : '%s'\n", currentCard.id, currentCard.front, currentCard.back);
        }

        // show cards

        deckDestroy(&existingDeck);
    }
    else // carder new New.deck
    {
        deck currentDeck = {0};
        currentDeck.name = argv[2];

        if (deckBuild(&currentDeck) == NULL)
        {
            return 2;
        }

        if (deckWrite(currentDeck) != 0)
        {
            return 3;
        }

        deckDestroy(&currentDeck);
    }

    return 0;
}