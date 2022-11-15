import ink

print('Once upon a time...')

index, val = ink.choice(
  'There were two choices.',
  'There were four lines of content.'
)

print(f'You selected {index}')

print('They lived happily ever after.')