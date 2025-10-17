// for (uint8_t i = 0; i < 26; i++)
//         {
//             printf("%c", key[i]);
//             if (i != 25)
//                 printf(", "); 
//         }
//         printf("\n\n");


// NUMBER_FLAG out;
//     for (size_t i = 3; i < 1000; i++)
//     {
//         EMRA(i,20,&out);
//         if(out == INCONCLUSIVE)
//             printf("%zu is prime\n", i);
//         else
//             printf("%zu is not prime\n", i);
//     }

/******************
 * START TEMPLATE *
 ******************/

/*

CY_STATE_FLAG nameCypher(const char *inpath, char *key, CRYPT crypt, const char *outpath)
{
    CY_STATE_FLAG state = NORMAL_CYPHER; CY_String file; char *buffer = NULL;

    state = cypherInit(inpath, &file, &buffer);
    if(state == CY_ERROR)
    {fprintf(stderr, "nameCypher(-> CY_ERROR <-)"); goto _freekey;}

    switch (crypt)
    {
        case ENCRYPTION:
        break;
        
        case DECRYPTION:
        break;
        
        default:
            fprintf(stderr, "No such parametre value nameCypher(..., CRYPT (-> %d <-), ...) No such parametre value", crypt);
            state = CY_ERROR;
            goto _free;
    }

    state = cypherEnd(outpath, &file, buffer);
    if(state == CY_ERROR)
        fprintf(stderr, "nameCypher(-> CY_ERROR <-)");
    goto _return;

    _free:
        free(file.str);
        free(buffer);
    _freekey:
        free(key);
    _return:
        return state;
}

*/

/****************
 * END TEMPLATE *
 ****************/

// CY_STATE_FLAG caesarCypher(const char *inpath, uint8_t keya, uint8_t keyb, CRYPT crypt, const char *outpath)
// {
//     CY_String file; char *buffer = NULL;
//     CY_STATE_FLAG state = cypherInit(inpath, &file, &buffer);
//     if(state == CY_ERROR)
//     {fprintf(stderr, "caesarCypher(-> CY_ERROR <-)"); goto _return;}

//     uint64_t keyainv;
//     if(EEA(keya, 26, &keyainv) == CY_ERROR)
//     {state = CY_ERROR; goto _free;}
//     keyb %= (uint8_t) 26;

//     switch (crypt)
//     {
//         case ENCRYPTION:
//             for (size_t i = 0; i < file.size; i++)
//             {
//                 char c = file.str[i];
//                 if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
//                 {
//                     char cas = 'a';
//                     if((c >= 'A' && c <= 'Z'))
//                         cas = 'A';
//                     c = ((keya * (c - cas) + keyb) % 26 + cas);
//                 }
//                 buffer[i] = c;
//             } 
//         break;
        
//         case DECRYPTION:
//             for (size_t i = 0; i < file.size; i++)
//             {
//                 char c = file.str[i];
//                 if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
//                 {
//                     char cas = 'a';
//                     if((c >= 'A' && c <= 'Z'))
//                         cas = 'A';
//                     c = ((keyainv * (c - cas - keyb + 26)) % 26 + cas);
//                 }
//                 buffer[i] = c;
//             } 
//         break;
        
//         default:
//             fprintf(stderr, "No such parametre value caesarCypher(..., CRYPT (-> %d <-), ...) No such parametre value", crypt);
//             state = CY_ERROR;
//             goto _free;
//     }

//     state = cypherEnd(outpath, &file, &buffer);
//     if(state == CY_ERROR)
//         fprintf(stderr, "caesarCypher(-> CY_ERROR <-)");
//     goto _return;

//     _free:
//         free(file.str);
//         free(buffer);

//     _return:
//         return state;
// }


// CY_STATE_FLAG monoalphabeticCypher(const char *inpath, char *key, CRYPT crypt, const char *outpath)
// {
//     CY_STATE_FLAG state = NORMAL_CYPHER; CY_String file; char *buffer = NULL; int ownkey = 0;

//     state = cypherInit(inpath, &file, &buffer);
//     if(state == CY_ERROR)
//     {fprintf(stderr, "monoalphabeticCypher(-> CY_ERROR <-)"); goto _freekey;}

//     if(key) 
//     {
//         ownkey = 1;
//     }

//     if(!ownkey) 
//     {
//     switch (crypt)
//     {
//         case ENCRYPTION:
//             key = malloc(26);
//             if (!key)
//             {perror("monoalphabeticCypher(-> malloc -> key <-)"); state = CY_ERROR; goto _return;}
//             for (uint8_t i = 0; i < 26; i++) key[i] = 'a' + i;
//             for (uint8_t i = 0; i < 26; i++)
//             {
//                 uint8_t r; 
//                 if(random_u8((uint8_t)(i + 1), &r) == CY_ERROR)
//                 {state = CY_ERROR; goto _freekey;}
//                 char tmp = key[i]; key[i] = key[r]; key[r] = tmp;
//             }
//         break;

//         case DECRYPTION:
//             key = malloc(26);
//             if (!key)
//             {perror("monoalphabeticCypher(-> malloc -> key <-)"); state = CY_ERROR; goto _return;}
//             size_t numtable[26] = {0};
//             for (size_t i = 0; i < file.size; i++)
//             {
//                 char c = file.str[i];
//                 if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
//                 {
//                     char cas = 'a';
//                     if((c >= 'A' && c <= 'Z'))
//                         cas = 'A';
//                     numtable[c - cas]++;
//                 }
//             }
//             char chartable[26];
//             for (size_t i = 0; i < 26; i++) chartable[i] = 'a' + i;
//             for (size_t i = 0; i < 26; i++)
//             {
//                 size_t k = i;
//                 for (size_t j = i + 1; j < 26; j++)
//                 {
//                     if(numtable[k] < numtable[j])
//                         k = j;
//                 }
//                 size_t tmpnum = numtable[i]; numtable[i] = numtable[k]; numtable[k] = tmpnum;
//                 char tmpchar = chartable[i]; chartable[i] = chartable[k]; chartable[k] = tmpchar;
//             }
//             char freqchar[] = "etaoinshrdlcumwfgypbvkjxqz";
//             for (size_t i = 0; i < 26; i++)
//                 key[chartable[i] - 'a'] = freqchar[i];                
//         break;
        
//         default:
//             fprintf(stderr, "No such parametre value monoalphabeticCypher(..., CRYPT (-> %d <-), ...) No such parametre value", crypt);
//             state = CY_ERROR;
//             goto _free;
//     }

//     }
//     for (size_t i = 0; i < file.size; i++)
//     {
//        char c = file.str[i];
//         if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
//         {
//             char cas = 'a';
//             if((c >= 'A' && c <= 'Z'))
//                 cas = 'A';
//             c = key[c - cas] - 'a' + cas;
//         }
//         buffer[i] = c; 
//     }

//     state = cypherEnd(outpath, &file, &buffer);
//     if(state == CY_ERROR)
//         fprintf(stderr, "monoalphabeticCypher(-> CY_ERROR <-)");
//     goto _return;

//     _free:
//         free(file.str);
//         free(buffer);
//     _freekey:
//         if(!ownkey) free(key);
//     _return:
//         return state;
// }